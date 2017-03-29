#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/qqml.h>
#include <QColor>
#include <QPalette>

class Theme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal horizontalPageMargin MEMBER m_horizontalPageMargin CONSTANT)
    Q_PROPERTY(qreal paddingLarge MEMBER m_paddingLarge CONSTANT)
    Q_PROPERTY(qreal paddingSmall MEMBER m_paddingSmall CONSTANT)
    Q_PROPERTY(int fontSizeMedium MEMBER m_fontSizeMedium CONSTANT)
    Q_PROPERTY(QColor primaryColor MEMBER m_primaryColor CONSTANT)
    Q_PROPERTY(QColor highlightColor MEMBER m_highlightColor CONSTANT)
    Q_PROPERTY(qreal itemSizeExtraSmall  MEMBER m_itemSizeExtraSmall CONSTANT)
    Q_PROPERTY(qreal itemSizeSmall  MEMBER m_itemSizeSmall CONSTANT)
    Q_PROPERTY(qreal itemSizeMedium MEMBER m_itemSizeMedium CONSTANT)
    Q_PROPERTY(qreal itemSizeLarge MEMBER m_itemSizeLarge CONSTANT)
    Q_PROPERTY(qreal itemSizeExtraLarge MEMBER m_itemSizeExtraLarge CONSTANT)

    Q_PROPERTY(qreal iconSizeSmall MEMBER m_iconSizeSmall CONSTANT)

    Q_PROPERTY(qreal buttonWidthSmall MEMBER m_buttonWidthSmall CONSTANT)
    Q_PROPERTY(qreal buttonWidthMedium MEMBER m_buttonWidthMedium CONSTANT)

    Q_PROPERTY(int longDuration MEMBER m_longDuration CONSTANT)

public:
    explicit Theme(QObject *parent = nullptr) :
        QObject(parent),
        m_fontSizeTiny(20),
        m_fontSizeExtraSmall(24),
        m_fontSizeSmall(28),
        m_fontSizeMedium(32),
        m_fontSizeLarge(40),
        m_fontSizeExtraLarge(50),
        m_fontSizeHuge(64),
        m_fontSizeTinyBase(20),
        m_fontSizeExtraSmallBase(24),
        m_fontSizeSmallBase(28),
        m_fontSizeMediumBase(32),
        m_fontSizeLargeBase(40),
        m_fontSizeExtraLargeBase(50),
        m_fontSizeHugeBase(64),
        m_itemSizeExtraSmall(70),
        m_itemSizeSmall(80),
        m_itemSizeMedium(100),
        m_itemSizeLarge(110),
        m_itemSizeExtraLarge(135),
        m_itemSizeHuge(180),
        m_iconSizeExtraSmall(24),
        m_iconSizeSmall(32),
        m_iconSizeSmallPlus(48),
        m_iconSizeMedium(64),
        m_iconSizeLarge(96),
        m_iconSizeExtraLarge(128),
        m_iconSizeLauncher(86),
        m_buttonWidthSmall(234),
        m_buttonWidthMedium(292),
        m_buttonWidthLarge(444),
        m_coverSizeSmall(148, 237),
        m_coverSizeLarge(234, 374),
        m_paddingSmall(6),
        m_paddingMedium(12),
        m_paddingLarge(24),
        m_horizontalPageMargin(m_paddingLarge),
        m_flickDeceleration(1500),
        m_maximumFlickVelocity(5000),
        m_pageStackIndicatorWidth(37),
        m_highlightBackgroundOpacity(0.3),
        m_longDuration(800)
    {
        const QPalette pal = QGuiApplication::palette();

        m_highlightColor = pal.highlightedText().color();
        m_primaryColor = pal.base().color();
        m_secondaryColor = pal.alternateBase().color();
        m_secondaryHighlightColor = pal.highlightedText().color();
        m_highlightBackgroundColor = pal.background().color();
    }

    QString m_fontFamilyHeading;
    QString m_fontFamily;
    QStringList m_launcherIconDirectories;
    int m_fontSizeTiny;
    int m_fontSizeExtraSmall;
    int m_fontSizeSmall;
    int m_fontSizeMedium;
    int m_fontSizeLarge;
    int m_fontSizeExtraLarge;
    int m_fontSizeHuge;
    int m_fontSizeTinyBase;
    int m_fontSizeExtraSmallBase;
    int m_fontSizeSmallBase;
    int m_fontSizeMediumBase;
    int m_fontSizeLargeBase;
    int m_fontSizeExtraLargeBase;
    int m_fontSizeHugeBase;
    qreal m_itemSizeExtraSmall;
    qreal m_itemSizeSmall;
    qreal m_itemSizeMedium;
    qreal m_itemSizeLarge;
    qreal m_itemSizeExtraLarge;
    qreal m_itemSizeHuge;
    qreal m_iconSizeExtraSmall;
    qreal m_iconSizeSmall;
    qreal m_iconSizeSmallPlus;
    qreal m_iconSizeMedium;
    qreal m_iconSizeLarge;
    qreal m_iconSizeExtraLarge;
    qreal m_iconSizeLauncher;
    qreal m_buttonWidthSmall;
    qreal m_buttonWidthMedium;
    qreal m_buttonWidthLarge;
    QSize m_coverSizeSmall;
    QSize m_coverSizeLarge;
    qreal m_paddingSmall;
    qreal m_paddingMedium;
    qreal m_paddingLarge;
    qreal m_horizontalPageMargin;
    qreal m_flickDeceleration;
    qreal m_maximumFlickVelocity;
    float m_pageStackIndicatorWidth;
    float m_highlightBackgroundOpacity;
    qreal m_pixelRatio;
    qreal m_webviewCustomLayoutWidthScalingFactor;
    int m_minimumPressHighlightTime;
    QColor m_highlightColor;
    QColor m_primaryColor;
    QColor m_secondaryColor;
    QColor m_secondaryHighlightColor;
    QColor m_highlightBackgroundColor;
    QColor m_highlightDimmerColor;
    int m_longDuration;

};

// Second, define the singleton type provider function (callback).
static QObject *them_type_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    Theme *theme = new Theme();
    return theme;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    qmlRegisterSingletonType<Theme>("TelegramQtTheme", 1, 0, "Theme", them_type_provider);

//    engine.addImportPath(app.applicationDirPath() + "../../imports/");
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

#include "main.moc"
