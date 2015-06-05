#ifndef ICOMMANDINVOKERLISTENER_H
#define ICOMMANDINVOKERLISTENER_H


class ICommandInvokerListener
{
public:
    ICommandInvokerListener() {}
    ~ICommandInvokerListener() {}

    virtual void OnNewMessage(QString &strMsg) = 0;
};

#endif // ICOMMANDINVOKERLISTENER_H
