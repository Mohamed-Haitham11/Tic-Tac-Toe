#include "tictactoe.h"

void TicTacToe::applyStyleSheet() {
    QString style;
    QString font;

    if (selectedTheme == "Dark") {
        font = "Segoe UI";
        style = QString(
                    "QWidget { background-color: #2D2D2D; color: #E0E0E0; font-family: '%1'; }"
                    "QLabel { color: #E0E0E0; }"
                    "QLineEdit, QSpinBox { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #5D5D5D; border-radius: 4px; padding: 5px; }"
                    "QPushButton { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #5D5D5D; border-radius: 4px; padding: 8px; }"
                    "QPushButton:hover { background-color: #4D4D4D; }"
                    "QPushButton:pressed { background-color: #5D5D5D; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #3D3D3D; color: #E0E0E0; border: 1px solid #555555; padding: 10px; font-family: 'Segoe UI'; }");
    }

    else if (selectedTheme == "Blue") {
        font = "Calibri";
        style = QString(
                    "QWidget { background-color: #1E1E40; color: #FFFFFF; font-family: '%1'; }"
                    "QLabel { color: #FFFFFF; }"
                    "QLineEdit, QSpinBox { background-color: #304070; color: #FFFFFF; border: 1px solid #506090; border-radius: 4px; padding: 5px; }"
                    "QPushButton { background-color: #304070; color: #FFFFFF; border: 1px solid #506090; border-radius: 4px; padding: 8px; }"
                    "QPushButton:hover { background-color: #4060A0; }"
                    "QPushButton:pressed { background-color: #5070C0; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #2B3A67; color: #FFFFFF; border: 1px solid #4060A0; padding: 10px; font-family: 'Calibri'; }");
    }

    else if (selectedTheme == "Plywood") {
        font = "Georgia";
        style = QString(
                    "QWidget { background-color: #D7C4A3; color: #4E342E; font-family: '%1'; }"
                    "QLabel { color: #4E342E; }"
                    "QLineEdit, QSpinBox { background-color: #EFEBE9; color: #4E342E; border: 1px solid #8D6E63; border-radius: 4px; padding: 5px; }"
                    "QPushButton { background-color: #BCAAA4; color: #3E2723; border: 1px solid #8D6E63; border-radius: 4px; padding: 8px; }"
                    "QPushButton:hover { background-color: #A1887F; }"
                    "QPushButton:pressed { background-color: #8D6E63; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #EFEBE9; color: #4E342E; border: 1px solid #8D6E63; padding: 10px; font-family: 'Georgia'; }");
    }

    else if (selectedTheme == "S.P.Q.R") {
        font = "Trajan Pro";
        style = QString(
                    "QWidget { background-color: #6E1414; color: #FFD700; font-family: '%1'; }"
                    "QLabel { color: #FFD700; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background-color: #9E1B1B; color: #FFD700; border: 1px solid #B22222; border-radius: 4px; padding: 5px; }"
                    "QPushButton { background-color: #800000; color: #FFD700; border: 1px solid #B22222; border-radius: 4px; padding: 8px; }"
                    "QPushButton:hover { background-color: #A52A2A; }"
                    "QPushButton:pressed { background-color: #B22222; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #9E1B1B; color: #FFD700; border: 1px solid #B22222; padding: 10px; font-family: 'Trajan Pro'; }");
    }

    else if (selectedTheme == "Carthago") {
        font = "Palatino Linotype";
        style = QString(
                    "QWidget { background-color: #3E1F47; color: #FFFFFF; font-family: '%1'; }"
                    "QLabel { color: #FFFFFF; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background-color: #5C2E7E; color: #FFFFFF; border: 1px solid #8A4FAD; border-radius: 4px; padding: 5px; }"
                    "QPushButton { background-color: #6A1B9A; color: #FFFFFF; border: 1px solid #9C4DCC; border-radius: 4px; padding: 8px; }"
                    "QPushButton:hover { background-color: #7E57C2; }"
                    "QPushButton:pressed { background-color: #9C4DCC; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #5C2E7E; color: #FFFFFF; border: 1px solid #9C4DCC; padding: 10px; font-family: 'Palatino Linotype'; }");
    }

    else if (selectedTheme == "Frosted Glass") {
        font = "Verdana";
        style = QString(
                    "QWidget { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #E0F7FA, stop:1 #B2EBF2); color: #003E57; font-family: '%1'; }"
                    "QLabel { color: #003E57; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background: rgba(255,255,255,0.7); color: #003E57; border: 1px solid #B0BEC5; border-radius: 6px; padding: 5px; }"
                    "QPushButton { background: rgba(255,255,255,0.8); color: #004D60; border: 1px solid #90A4AE; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background: rgba(255,255,255,0.9); }"
                    "QPushButton:pressed { background: #CFD8DC; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background: rgba(255,255,255,0.6); color: #004D60; border: 1px solid #B0BEC5; padding: 10px; font-family: 'Verdana'; }");
    }

    else if (selectedTheme == "Ancient Egypt") {
        font = "Papyrus";
        style = QString(
                    "QWidget { background-color: #F5E5B8; color: #5C432E; font-family: '%1'; }"
                    "QLabel { color: #5C432E; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background-color: #F9F3D2; color: #5C432E; border: 1px solid #CBB67C; border-radius: 5px; padding: 5px; }"
                    "QPushButton { background-color: #E5C56E; color: #4E342E; border: 1px solid #B79850; border-radius: 5px; padding: 8px; }"
                    "QPushButton:hover { background-color: #DFC276; }"
                    "QPushButton:pressed { background-color: #CBB67C; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #F9F3D2; color: #5C432E; border: 1px solid #CBB67C; padding: 10px; font-family: 'Papyrus'; }");
    }

    else if (selectedTheme == "Seljuk Empire") {
        font = "Noto Naskh Arabic";
        style = QString(
                    "QWidget { background-color: #0D3B66; color: #FAF0E6; font-family: '%1'; }"
                    "QLabel { color: #FAF0E6; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background-color: #145DA0; color: #FFFFFF; border: 1px solid #1E81B0; border-radius: 6px; padding: 5px; }"
                    "QPushButton { background-color: #1E81B0; color: #FFFFFF; border: 1px solid #63A4FF; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #63A4FF; }"
                    "QPushButton:pressed { background-color: #145DA0; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #145DA0; color: #FFFFFF; border: 1px solid #63A4FF; padding: 10px; font-family: 'Noto Naskh Arabic'; }");
    }

    else if (selectedTheme == "Cyber Enhanced") {
        font = "Consolas";
        style = QString(
                    "QWidget { background-color: #0F0F0F; color: #00FFEA; font-family: '%1'; }"
                    "QLabel { color: #00FFEA; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background-color: #1F1F1F; color: #00FFEA; border: 1px solid #00FFEA; border-radius: 6px; padding: 6px; }"
                    "QPushButton { background-color: #1F1F1F; color: #00FFEA; border: 1px solid #00FFEA; border-radius: 6px; padding: 8px; }"
                    "QPushButton:hover { background-color: #00FFEA; color: #0F0F0F; }"
                    "QPushButton:pressed { background-color: #0F0F0F; color: #00FFEA; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #1F1F1F; color: #00FFEA; border: 1px solid #00FFEA; padding: 10px; font-family: 'Consolas'; }");
    }

    else if (selectedTheme == "8-Bit") {
        font = "Courier New";
        style = QString(
                    "QWidget { background-color: #282828; color: #FFD700; font-family: '%1'; }"
                    "QLabel { color: #FFD700; font-weight: bold; }"
                    "QLineEdit, QSpinBox { background-color: #404040; color: #FFD700; border: 2px solid #FFD700; border-radius: 0px; padding: 6px; }"
                    "QPushButton { background-color: #404040; color: #FFD700; border: 2px solid #FFD700; padding: 8px; }"
                    "QPushButton:hover { background-color: #606060; }"
                    "QPushButton:pressed { background-color: #303030; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #303030; color: #FFD700; border: 2px solid #FFD700; padding: 10px; font-family: 'Courier New'; }");
    }

    else {
        // Default Light Theme
        font = "Arial";
        style = QString(
                    "QWidget { background-color: #F5F5F5; color: #333333; font-family: '%1'; }"
                    "QLabel { color: #333333; }"
                    "QLineEdit, QSpinBox { background-color: #FFFFFF; color: #333333; border: 1px solid #CCCCCC; border-radius: 4px; padding: 5px; }"
                    "QPushButton { background-color: #FFFFFF; color: #333333; border: 1px solid #CCCCCC; border-radius: 4px; padding: 8px; }"
                    "QPushButton:hover { background-color: #EEEEEE; }"
                    "QPushButton:pressed { background-color: #DDDDDD; }"
                    ).arg(font);
        scoreLabel->setStyleSheet("QLabel { background-color: #F0F0F0; color: #333333; border: 1px solid #CCCCCC; padding: 10px; font-family: 'Arial'; }");
    }

    setStyleSheet(style);

    // Button text style (uniform across themes)
    for (int i = 0; i < 9; ++i) {
        QPushButton *button = qobject_cast<QPushButton *>(buttonGroup->button(i));
        if (button) {
            button->setStyleSheet(button->styleSheet() + QString(" QPushButton { font-weight: bold; font-size: 24px; padding: 10px; font-family: '%1'; }").arg(font));
        }
    }

    // Logo loading
    QLabel *logo = findChild<QLabel*>("themeLogo");
    if (logo) {
        QString themeKey = selectedTheme.toLower().replace(" ", "_").replace(".", "");
        QString logoPath = QString(":/logos/%1.png").arg(themeKey);
        QPixmap pixmap(logoPath);
        logo->setPixmap(pixmap.scaled(logo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
