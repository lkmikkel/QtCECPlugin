
#include <QtGui/qgenericplugin.h>
#include "qtcecagent.h"

QT_BEGIN_NAMESPACE

class QtCECPlugin : public QGenericPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QGenericPluginFactoryInterface_iid FILE "configure.json")

public:
    QtCECPlugin();

    QObject* create(const QString &key, const QString &specification) Q_DECL_OVERRIDE;
};

QtCECPlugin::QtCECPlugin()
    : QGenericPlugin()
{
}

QObject* QtCECPlugin::create(const QString &key,
                                                 const QString &specification)
{
    Q_UNUSED(specification);
    if (!key.compare(QLatin1String("QtCECAgent"), Qt::CaseInsensitive))
        return new QtCECAgent();
    return 0;
}

QT_END_NAMESPACE

#include "main.moc"
