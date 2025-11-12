#include "tgtsmlplatforminputcontext.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

TgtsmlPlatformInputContext::TgtsmlPlatformInputContext()
{
    m_focusitem = nullptr;
    m_keyboard = nullptr;
    static int count = 0;
    qDebug() << "Install count: " << ++count;
    qApp->installEventFilter(this);
}

TgtsmlPlatformInputContext::~TgtsmlPlatformInputContext()
{
    disconnect(m_keyboard, &KeyboardForm::sendKeyToFocusItem, this, &TgtsmlPlatformInputContext::sendKeyToFocusItem);
    if(m_keyboard) delete m_keyboard;
}

void TgtsmlPlatformInputContext::sendKeyToFocusItem(const QString &keytext)
{
    if(!m_focusitem)return;

    if(keytext == QString("\x7F"))     //Backspace <--
    {
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier));
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier));
    }
    else if(keytext == QString("\n"))
    {
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier));
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier));
    }
    else if(keytext == QString("&&"))
    {
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyPress, 0, Qt::NoModifier, "&"));
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyRelease, 0, Qt::NoModifier, "&"));
    }
    else
    {
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyPress, 0, Qt::NoModifier, keytext));
        QCoreApplication::sendEvent(m_focusitem, new QKeyEvent(QEvent::KeyRelease, 0, Qt::NoModifier, keytext));
    }
}

bool TgtsmlPlatformInputContext::isValid() const
{
    return true;
}

void TgtsmlPlatformInputContext::setFocusObject(QObject *object)
{
    m_focusitem = object;
}

void TgtsmlPlatformInputContext::showInputPanel()
{
    if(!m_keyboard){
        m_keyboard = new KeyboardForm;
        connect(m_keyboard, &KeyboardForm::sendKeyToFocusItem, this, &TgtsmlPlatformInputContext::sendKeyToFocusItem);
    }
    if(m_keyboard->isHidden())m_keyboard->show();
    QWidget *widgetTmp = qobject_cast<QWidget*>(m_focusitem);
    if(widgetTmp){
        QPoint widgetGlobalPos = widgetTmp->mapToGlobal(QPoint(0, 0));
        if(widgetGlobalPos.x() < 0){
            widgetGlobalPos.setX(0);
        }
        if(widgetGlobalPos.y() < 0){
            widgetGlobalPos.setY(0);
        }
        if(qApp->desktop()->width() - widgetGlobalPos.x() < m_keyboard->width()){
            widgetGlobalPos.setX(qApp->desktop()->width() - m_keyboard->width());
        }
        if(qApp->desktop()->height() - widgetGlobalPos.y() - 30 < m_keyboard->height()){
            widgetGlobalPos.setY(widgetGlobalPos.y() - m_keyboard->height() - 10);
        }
        else{
             widgetGlobalPos = widgetGlobalPos + QPoint(0,30);
        }
        m_keyboard->move(widgetGlobalPos);
    }
}

void TgtsmlPlatformInputContext::hideInputPanel()
{
    if(!m_keyboard){
        return;
    }
    if(!m_keyboard->isHidden()){
        m_keyboard->hide();
    }
    m_keyboard->clearChineseCache();
}

bool TgtsmlPlatformInputContext::isInputPanelVisible() const
{
    return m_keyboard->isVisible();
}
#include <QPushButton>
bool TgtsmlPlatformInputContext::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress ) {
        const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
        QString text = keyEvent->text().toLower();  // 小写字符

        // 检查是否是键盘窗口发来的事件
           if (text.isEmpty() || QString(obj->metaObject()->className()) != "QWidgetWindow")
               return false;

           if (!m_keyboard || !m_keyboard->isVisible())
               return false;

           // 处理退格特殊符号
           qDebug() <<text;
           QString keyToMatch = (text == "\b") ? "\uF060" : text;

           // 查找所有按钮，只在第一次匹配时执行点击
           const QList<QPushButton *> btns = m_keyboard->findChildren<QPushButton *>();
           for (QPushButton *btn : btns) {
                qDebug() <<btn->text();
               if (btn->text().toLower() == keyToMatch) {
                   qDebug() << "Simulate click for key:" << keyToMatch;
                   //btn->click();
                   btn->animateClick();
                   return true;  // 拦截该事件
               }
           }

        return false;  // 返回 false 表示事件继续传递
    }

    return false;
}

