#include "qtcecagent.h"

// required by cecloader.h
#include <iostream>
#include <libcec/cec.h>
#include <libcec/cecloader.h>

#include <QtCore/QLoggingCategory>
#include <qpa/qwindowsysteminterface.h>

#include <QGuiApplication>

using namespace CEC;

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(ceclog, "qt.ceckeyboard")

#define MAX_CEC_DEVICES 10

static void CECLogMessageCallback(void *adapter, const cec_log_message* message);
static void CECKeyPressCallback(void *adapter, const cec_keypress* keypress);
static void CECCommandCallback(void *adapter, const cec_command* command);
static void CECAlertCallback(void *adapter, const libcec_alert alert, const libcec_parameter data);
static void CECSourceActivatedCallback(void *adapter, const cec_logical_address address, const uint8_t activated);

QtCECAgent::QtCECAgent(QObject *p)
    : QObject(p),
      cec_adapter(Q_NULLPTR)
{
    CEC::libcec_configuration cec_config;
    cec_config.Clear();

    QString cecAdvertizedName = "QPi";

    if (!qApp->applicationName().isNull())
        cecAdvertizedName = qApp->applicationName();

    const std::string devicename(cecAdvertizedName.toLocal8Bit().constData());
    devicename.copy(cec_config.strDeviceName, 13u);

    cec_config.clientVersion = CEC::LIBCEC_VERSION_CURRENT;
    cec_config.bActivateSource = 0;
    cec_config.callbacks = &cec_callbacks;
    cec_config.deviceTypes.Add(CEC::CEC_DEVICE_TYPE_RECORDING_DEVICE);
    cec_config.callbackParam = this;

    cec_callbacks.Clear();
    cec_callbacks.alert = &CECAlertCallback;
    cec_callbacks.keyPress = &CECKeyPressCallback;
    cec_callbacks.logMessage = &CECLogMessageCallback;
    cec_callbacks.commandReceived = &CECCommandCallback;
    cec_callbacks.sourceActivated = &CECSourceActivatedCallback;

    cec_adapter = LibCecInitialise(&cec_config);

    if(!cec_adapter) {
        qCCritical(ceclog).noquote() << "Could not create CEC adaptor with current config";
        return;
    }

    // initialize 
    cec_adapter->InitVideoStandalone();

    cec_adapter_descriptor devices[MAX_CEC_DEVICES];
    uint8_t devices_found = cec_adapter->DetectAdapters(devices, MAX_CEC_DEVICES, NULL, true);
    if(devices_found < 1) {
        qCWarning(ceclog).noquote() << "No CEC devices found";
        close();
	return;
    }

    if(!cec_adapter->Open(devices[0].strComName)) {
        qCCritical(ceclog).noquote() << "Can't open device 0 (assumed to be TV)";
        close();
	return;
    }

    qCDebug(ceclog).noquote() << "Successfully created Qt CEC input plugin";
}

QtCECAgent::~QtCECAgent() {
    close();
}

void QtCECAgent::close()
{
    if (cec_adapter) {
        qCDebug(ceclog).noquote() << "Closing the CEC device";
    	cec_adapter->Close();
    	UnloadLibCec(cec_adapter);
    }
    cec_adapter = Q_NULLPTR;
}

QString QtCECAgent::addressToString(cec_logical_address a)
{
    if (cec_adapter) {
        return QString(cec_adapter->ToString(a));
    } else {
        return QString();
    }
}

static void CECAlertCallback(void *adapter, const libcec_alert alert, const libcec_parameter data)
{
    Q_UNUSED(adapter)
    QString param;
    switch (data.paramType)
    {
        case CEC_PARAMETER_TYPE_STRING:
            param = QString(": %1").arg((char*)data.paramData);
            break;
        case CEC_PARAMETER_TYPE_UNKOWN:
        default:
            if (data.paramData != NULL)
            {
                param = QString(": UNKNOWN param has type %1").arg(data.paramType);
            }
            break;
    }

    switch (alert)
      {
          case CEC_ALERT_SERVICE_DEVICE:
              qCWarning(ceclog).noquote() << "CEC alert device service message" << param;
              break;
          case CEC_ALERT_CONNECTION_LOST:
              qCDebug(ceclog).noquote() << "CEC device connection lost" << param;
              break;
          case CEC_ALERT_PERMISSION_ERROR:
          case CEC_ALERT_PORT_BUSY:
              break;
          case CEC_ALERT_PHYSICAL_ADDRESS_ERROR:
              qCCritical(ceclog).noquote() << "CEC physical address error" << param;
              break;
          case CEC_ALERT_TV_POLL_FAILED:
              qCritical(ceclog).noquote() << "CEC alert device can't poll TV" << param;
              break;
          default:
              qCDebug(ceclog).noquote() << "UNKNOWN CEC device alert " << alert << param;
              break;
      }
}

static void CECLogMessageCallback(void *adapter, const cec_log_message* message)
{
    Q_UNUSED(adapter)
    if (message && message->message)
    {
        qCDebug(ceclog).noquote() << "LOG:" << QString(message->message);
    }
}

static void CECKeyPressCallback(void* adapter, const cec_keypress* msg)
{
    Q_UNUSED(adapter);
    // Mapping CEC keys to Qt keys

    Qt::Key key = Qt::Key(0);
    // nativeKeyCode's manually looked up via xev
    int nativeKeyCode = -1;
    switch(msg->keycode)
    {
    case CEC::CEC_USER_CONTROL_CODE_PLAY:
        key = Qt::Key_MediaPlay;
        nativeKeyCode = 172;
        break;
    case CEC::CEC_USER_CONTROL_CODE_STOP:
        key = Qt::Key_MediaStop;
        nativeKeyCode = 174;
        break;
    case CEC::CEC_USER_CONTROL_CODE_RECORD:
        key = Qt::Key_MediaRecord;
        nativeKeyCode = 120;
        break;
    case CEC::CEC_USER_CONTROL_CODE_REWIND:
        key = Qt::Key_MediaPrevious;
        nativeKeyCode = 173;
        break;
    case CEC::CEC_USER_CONTROL_CODE_FAST_FORWARD:
        key = Qt::Key_MediaNext;
        nativeKeyCode = 171;
        break;
    case CEC::CEC_USER_CONTROL_CODE_SELECT:
        key = Qt::Key_Select;
        nativeKeyCode = 36;
        break;
    case CEC::CEC_USER_CONTROL_CODE_ENTER:
        key = Qt::Key_Enter;
        nativeKeyCode = 36;
        break;
    case CEC::CEC_USER_CONTROL_CODE_DISPLAY_INFORMATION:
        key = Qt::Key_Info;
        nativeKeyCode = 0;
        break;
    case CEC::CEC_USER_CONTROL_CODE_UP:
        key = Qt::Key_Up;
        nativeKeyCode = 111;
        break;
    case CEC::CEC_USER_CONTROL_CODE_DOWN:
        key = Qt::Key_Down;
        nativeKeyCode = 116;
        break;
    case CEC::CEC_USER_CONTROL_CODE_LEFT:
        key = Qt::Key_Left;
        nativeKeyCode = 113;
        break;
    case CEC::CEC_USER_CONTROL_CODE_RIGHT:
        key = Qt::Key_Right;
        nativeKeyCode = 114;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER0:
        key = Qt::Key_0;
        nativeKeyCode = 19;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER1:
        key = Qt::Key_1;
        nativeKeyCode = 10;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER2:
        key = Qt::Key_2;
        nativeKeyCode = 11;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER3:
        key = Qt::Key_3;
        nativeKeyCode = 12;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER4:
        key = Qt::Key_4;
        nativeKeyCode = 13;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER5:
        key = Qt::Key_5;
        nativeKeyCode = 14;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER6:
        key = Qt::Key_6;
        nativeKeyCode = 15;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER7:
        key = Qt::Key_7;
        nativeKeyCode = 16;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER8:
        key = Qt::Key_8;
        nativeKeyCode = 17;
        break;
    case CEC::CEC_USER_CONTROL_CODE_NUMBER9:
        key = Qt::Key_9;
        nativeKeyCode = 18;
        break;
    case CEC::CEC_USER_CONTROL_CODE_F1_BLUE:
        key = Qt::Key_F1;
        nativeKeyCode = 67;
        break;
    case CEC::CEC_USER_CONTROL_CODE_F2_RED:
        key = Qt::Key_F2;
        nativeKeyCode = 68;
        break;
    case CEC::CEC_USER_CONTROL_CODE_F3_GREEN:
        key = Qt::Key_F3;
        nativeKeyCode = 69;
        break;
    case CEC::CEC_USER_CONTROL_CODE_F4_YELLOW:
        key = Qt::Key_F4;
        nativeKeyCode = 70;
        break;
    case CEC::CEC_USER_CONTROL_CODE_CHANNEL_UP:
        key = Qt::Key_PageUp;
        nativeKeyCode = 112;
        break;
    case CEC::CEC_USER_CONTROL_CODE_CHANNEL_DOWN:
        key = Qt::Key_PageDown;
        nativeKeyCode = 117;
        break;
    case CEC::CEC_USER_CONTROL_CODE_EXIT:
        key = Qt::Key_Escape;
        nativeKeyCode = 9;
        break;
    case CEC::CEC_USER_CONTROL_CODE_AN_RETURN:
        key = Qt::Key_Backspace;
        nativeKeyCode = 22;
        break;
    default: break;
    };

    if (key) {
        QWindowSystemInterface::handleExtendedKeyEvent(0,
                                                       (msg->duration ? QEvent::KeyRelease : QEvent::KeyPress),
                                                       key,
                                                       0,
                                                       nativeKeyCode,
                                                       0,
                                                       0,
                                                       QString(),
                                                       false);
    } else {
        qCWarning(ceclog).noquote() << "CEC key press" << msg->keycode << "not handled by the Qt CEC plugin";
    }

}

static void CECCommandCallback(void *adapter, const cec_command* command)
{
    Q_UNUSED(adapter)
    Q_UNUSED(command)
}

static void CECSourceActivatedCallback(void *adapter, const cec_logical_address address, const uint8_t activated)
{
    qCDebug(ceclog).noquote() << "CEC Source" << ((QCECKeyboardManager*) adapter)->addressToString(address) << (activated ? "Ativated" : "Deactivated");
}

QT_END_NAMESPACE
