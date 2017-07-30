#ifndef QT_CEC_AGENT_H_
#define QT_CEC_AGENT_H_

#include <QObject>
#include <libcec/cectypes.h>

namespace CEC {
    class ICECAdapter;
};

QT_BEGIN_NAMESPACE

class QtCECAgent : public QObject {
    Q_OBJECT
public:
    QtCECAgent(QObject *p = 0);
    ~QtCECAgent();

    void close();
    QString addressToString(CEC::cec_logical_address a);
private:
    CEC::ICECAdapter* cec_adapter;
    CEC::ICECCallbacks cec_callbacks;
};

QT_END_NAMESPACE

#endif // QT_CEC_AGENT_H_
