#include "radio.h"
#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <QMediaService>
#include <QAudioOutputSelectorControl>

Radio::Radio()
    : base_path("/media/usb0"),
      current_path("/media/usb0")
{
    window = new QWidget;
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    window->setAutoFillBackground(true);
    window->setPalette(pal);

    /* Menu buttons on the left side */
    radio_btn = new QPushButton;
    radio_btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    radio_btn->setText(tr("Radio"));
    mp3_btn = new QPushButton;
    mp3_btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mp3_btn->setText(tr("MP3"));
    config_btn = new QPushButton;
    config_btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    config_btn->setText(tr("Config"));
    quit_btn = new QPushButton;
    quit_btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    quit_btn->setText(tr("Quit"));
    connect(quit_btn, SIGNAL(clicked()), this , SLOT(quit_button_press()));

    menu_buttons_layout = new QVBoxLayout;
    menu_buttons_layout->addWidget(radio_btn);
    menu_buttons_layout->addWidget(mp3_btn);
    menu_buttons_layout->addWidget(config_btn);
    menu_buttons_layout->addWidget(quit_btn);

    /* Buttons on the bottom part */
    prev_btn = new custombutton("://images/idle.png", "://images/pressed.png");
    prev_btn->setText(tr("prev"));
    play_btn = new custombutton("://images/idle.png", "://images/pressed.png");
    play_btn->setText(tr("play"));
    stop_btn = new custombutton("://images/idle.png", "://images/pressed.png");
    stop_btn->setText(tr("stop"));
    next_btn = new custombutton("://images/idle.png", "://images/pressed.png");
    next_btn->setText(tr("next"));

    bottom_buttons_layout = new QHBoxLayout;
    bottom_buttons_layout->addWidget(prev_btn);
    bottom_buttons_layout->addWidget(play_btn);
    bottom_buttons_layout->addWidget(stop_btn);
    bottom_buttons_layout->addWidget(next_btn);

    list_view = new QListWidget(this);
    list_view->setAttribute(Qt::WA_AcceptTouchEvents,true);
    list_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list_view->setStyleSheet("QListWidget{ background-color: black; color: white; }");
    connect(list_view, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(on_list_item_clicked(QListWidgetItem*)));

    QScrollerProperties scroller_properties;
    scroller_properties.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.6);
    scroller_properties.setScrollMetric(QScrollerProperties::MinimumVelocity, 0.0);
    scroller_properties.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.5);
    scroller_properties.setScrollMetric(QScrollerProperties::AcceleratingFlickMaximumTime, 0.4);
    scroller_properties.setScrollMetric(QScrollerProperties::AcceleratingFlickSpeedupFactor, 1.2);
    scroller_properties.setScrollMetric(QScrollerProperties::SnapPositionRatio, 0.2);
    scroller_properties.setScrollMetric(QScrollerProperties::MaximumClickThroughVelocity, 0);
    scroller_properties.setScrollMetric(QScrollerProperties::DragStartDistance, 0.001);
    scroller_properties.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.5);
    scroller = QScroller::scroller(list_view);
    scroller->setScrollerProperties(scroller_properties);
    scroller->grabGesture(list_view, QScroller::LeftMouseButtonGesture);

    right_layout = new QVBoxLayout;
    right_layout->addWidget(list_view);
    right_layout->addLayout(bottom_buttons_layout);

    /* Main layout */
    main_layout = new QHBoxLayout;
    main_layout->addLayout(menu_buttons_layout);
    main_layout->addLayout(right_layout);

    window->setLayout(main_layout);
    setCentralWidget(window);
    window->show();

    update_list();
}

Radio::~Radio()
{
}

void Radio::quit_button_press()
{
    std::cout << "Quitting....\n";
    exit(0);
}

void Radio::radio_button_press()
{

}

void Radio::update_list()
{
    list_view->clear();

    QListWidgetItem* list_item = new QListWidgetItem("dummy");
    QFont curr_font = list_item->font();
    curr_font.setPointSize(24);

    DIR *dir = opendir(current_path.toUtf8());
    if (dir == nullptr) {
        std::cout << "Error: wrong path " << current_path.toStdString() << std::endl;
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if ((strcmp(entry->d_name, "..") == 0) || (entry->d_name[0] != '.')) {
            list_item = new QListWidgetItem(tr(entry->d_name), list_view);
            list_item->setFont(curr_font);
        } else {
            std::cout << "Skipping item = " << entry->d_name << std::endl;
        }
    }
    closedir(dir);
}

void Radio::on_list_item_clicked(QListWidgetItem* list_widget_item)
{
    QString selected_path = current_path + "/" + list_widget_item->text();
    std::cout << "Selected = " << selected_path.toStdString() << std::endl;

    struct stat file_stat;
    if( stat(selected_path.toUtf8(), &file_stat) == 0 ) {
        if( S_ISDIR(file_stat.st_mode)) {
            current_path = selected_path;
            std::cout << "Entering in directory" << std::endl;
            update_list();
        } else if(S_ISREG(file_stat.st_mode)) {
            std::cout << "Playing file" << std::endl;
            QMediaPlayer* player = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);
            ////
            QMediaService *svc = player->service();
            if (svc != NULL) {
                QAudioOutputSelectorControl *out = qobject_cast<QAudioOutputSelectorControl *> (svc->requestControl(QAudioOutputSelectorControl_iid));
                if (out != NULL) {
                    QList<QString> available_outputs = out->availableOutputs();
                    for (int i=0; i<available_outputs.length(); i++) {
                        std::cout << "Output = " << available_outputs[i].toStdString() << std::endl;
                    }
                } else {
                    std::cout << "QAudioOutputSelectorControl is null" << std::endl;
                }
            } else {
                std::cout << "QMediaService is null" << std::endl;
            }
            ////
            player->setMedia(QUrl::fromLocalFile(selected_path));
            player->setVolume(80);
            player->play();
        } else {
            std::cout << "Error: unknown element" << std::endl;
        }
    } else {
        std::cout << "Error while getting selection type" << std::endl;
    }
}

