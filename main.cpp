
#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <QColor>
#include <QDoubleValidator>
#include <QStringList>
#include <algorithm>
#include <cmath>

class Window : public QWidget {
    QLineEdit *c, *m, *y, *k;
    QLineEdit *r, *g, *b;
    QLineEdit *h, *s, *v;

    QSlider *sc, *sm, *sy, *sk;
    QSlider *sr, *sg, *sb;
    QSlider *sh, *ss, *sv;

    QLabel *preview;
    bool updating = false;

    QLineEdit* field(double value, double maxValue) {
        auto *edit = new QLineEdit;
        edit->setAlignment(Qt::AlignCenter);
        edit->setFixedWidth(75);
        edit->setValidator(new QDoubleValidator(
            0, maxValue, 2, edit));
        edit->setText(QString::number(value));
        return edit;
    }

    QSlider* slider(int maxValue) {
        auto *s = new QSlider(Qt::Horizontal);
        s->setRange(0, maxValue);
        return s;
    }

    QWidget* makePanel(const QString &title,
                       const QStringList &names,
                       QLineEdit **edits,
                       QSlider **bars) {
        auto *box = new QGroupBox(title);
        auto *grid = new QGridLayout(box);
        grid->setColumnStretch(2, 1);
        grid->setVerticalSpacing(10);

        for (int i = 0; i < names.size(); ++i) {
            grid->addWidget(new QLabel(names[i]), i, 0);
            grid->addWidget(edits[i], i, 1);
            grid->addWidget(bars[i], i, 2);
        }

        return box;
    }

    void showColor(int R, int G, int B,
                   bool updateCMYK, bool updateHSV) {
        R = std::clamp(R, 0, 255);
        G = std::clamp(G, 0, 255);
        B = std::clamp(B, 0, 255);

        double rr = R / 255.0;
        double gg = G / 255.0;
        double bb = B / 255.0;

        double mx = std::max({rr, gg, bb});
        double mn = std::min({rr, gg, bb});
        double d = mx - mn;

        double K = 1 - mx;
        double C = mx == 0 ? 0 : (1 - rr - K) / (1 - K);
        double M = mx == 0 ? 0 : (1 - gg - K) / (1 - K);
        double Y = mx == 0 ? 0 : (1 - bb - K) / (1 - K);

        double H = 0;

        if (d != 0) {
            if (mx == rr)
                H = 60 * fmod((gg - bb) / d, 6);
            else if (mx == gg)
                H = 60 * ((bb - rr) / d + 2);
            else
                H = 60 * ((rr - gg) / d + 4);
        }

        if (H < 0)
            H += 360;

        double S = mx == 0 ? 0 : d / mx;
        double V = mx;

        updating = true;

        r->setText(QString::number(R));
        g->setText(QString::number(G));
        b->setText(QString::number(B));

        sr->setValue(R);
        sg->setValue(G);
        sb->setValue(B);

        if (updateCMYK) {
            c->setText(QString::number(C * 100, 'f', 1));
            m->setText(QString::number(M * 100, 'f', 1));
            y->setText(QString::number(Y * 100, 'f', 1));
            k->setText(QString::number(K * 100, 'f', 1));

            sc->setValue(qRound(C * 100));
            sm->setValue(qRound(M * 100));
            sy->setValue(qRound(Y * 100));
            sk->setValue(qRound(K * 100));
        }

        if (updateHSV) {
            h->setText(QString::number(H, 'f', 1));
            s->setText(QString::number(S * 100, 'f', 1));
            v->setText(QString::number(V * 100, 'f', 1));

            sh->setValue(qRound(H));
            ss->setValue(qRound(S * 100));
            sv->setValue(qRound(V * 100));
        }

        preview->setStyleSheet(
            QString("background-color: rgb(%1,%2,%3);"
                    "border: 1px solid #999; border-radius: 8px;")
                .arg(R).arg(G).arg(B));

        updating = false;
    }

    void setRGB(int R, int G, int B) {
        showColor(R, G, B, true, true);
    }

    void fromCMYK() {
        double C = std::clamp(c->text().toDouble(), 0.0, 100.0) / 100;
        double M = std::clamp(m->text().toDouble(), 0.0, 100.0) / 100;
        double Y = std::clamp(y->text().toDouble(), 0.0, 100.0) / 100;
        double K = std::clamp(k->text().toDouble(), 0.0, 100.0) / 100;

        int R = qRound(255 * (1 - C) * (1 - K));
        int G = qRound(255 * (1 - M) * (1 - K));
        int B = qRound(255 * (1 - Y) * (1 - K));

        showColor(R, G, B, false, true);

        updating = true;

        c->setText(QString::number(C * 100, 'f', 1));
        m->setText(QString::number(M * 100, 'f', 1));
        y->setText(QString::number(Y * 100, 'f', 1));
        k->setText(QString::number(K * 100, 'f', 1));

        sc->setValue(qRound(C * 100));
        sm->setValue(qRound(M * 100));
        sy->setValue(qRound(Y * 100));
        sk->setValue(qRound(K * 100));

        updating = false;
    }

    void fromHSV() {
        double H = std::clamp(h->text().toDouble(), 0.0, 360.0);
        double S = std::clamp(s->text().toDouble(), 0.0, 100.0) / 100;
        double V = std::clamp(v->text().toDouble(), 0.0, 100.0) / 100;

        double C = V * S;
        double X = C * (1 - fabs(fmod(H / 60, 2) - 1));
        double M = V - C;

        double R = 0, G = 0, B = 0;

        if (H < 60)
            R = C, G = X;
        else if (H < 120)
            R = X, G = C;
        else if (H < 180)
            G = C, B = X;
        else if (H < 240)
            G = X, B = C;
        else if (H < 300)
            R = X, B = C;
        else
            R = C, B = X;

        int red = qRound((R + M) * 255);
        int green = qRound((G + M) * 255);
        int blue = qRound((B + M) * 255);

        showColor(red, green, blue, true, false);

        updating = true;

        h->setText(QString::number(H, 'f', 1));
        s->setText(QString::number(S * 100, 'f', 1));
        v->setText(QString::number(V * 100, 'f', 1));

        sh->setValue(qRound(H));
        ss->setValue(qRound(S * 100));
        sv->setValue(qRound(V * 100));

        updating = false;
    }

public:
    Window() {
        setWindowTitle("Цветовые модели");
        resize(850, 500);

        auto *main = new QVBoxLayout(this);
        auto *top = new QHBoxLayout;

        preview = new QLabel("Предпросмотр");
        preview->setMinimumSize(180, 100);
        preview->setAlignment(Qt::AlignCenter);

        auto *choose = new QPushButton("Выбрать цвет");

        auto *left = new QVBoxLayout;
        left->addWidget(preview);
        left->addWidget(choose);
        top->addLayout(left, 1);

        auto *models = new QGridLayout;
        top->addLayout(models, 3);
        main->addLayout(top);

        QLineEdit *rgb[] = {
            r = field(0, 255),
            g = field(0, 255),
            b = field(0, 255)
        };

        QSlider *rgbS[] = {
            sr = slider(255),
            sg = slider(255),
            sb = slider(255)
        };

        QLineEdit *cmyk[] = {
            c = field(0, 100),
            m = field(0, 100),
            y = field(0, 100),
            k = field(0, 100)
        };

        QSlider *cmykS[] = {
            sc = slider(100),
            sm = slider(100),
            sy = slider(100),
            sk = slider(100)
        };

        QLineEdit *hsv[] = {
            h = field(0, 360),
            s = field(0, 100),
            v = field(0, 100)
        };

        QSlider *hsvS[] = {
            sh = slider(360),
            ss = slider(100),
            sv = slider(100)
        };

        models->addWidget(
            makePanel("RGB", {"R", "G", "B"}, rgb, rgbS), 0, 0);

        models->addWidget(
            makePanel("CMYK", {"C", "M", "Y", "K"}, cmyk, cmykS), 0, 1);

        models->addWidget(
            makePanel("HSV", {"H", "S", "V"}, hsv, hsvS), 1, 0, 1, 2);

        auto connectFields = [this](
            QLineEdit **edits, int count, auto action) {
            for (int i = 0; i < count; ++i) {
                connect(edits[i], &QLineEdit::editingFinished, this,
                    [this, action] {
                        if (!updating)
                            action();
                    });
            }
        };

        connectFields(rgb, 3, [this] {
            setRGB(r->text().toInt(), g->text().toInt(), b->text().toInt());
        });

        connectFields(cmyk, 4, [this] {
            fromCMYK();
        });

        connectFields(hsv, 3, [this] {
            fromHSV();
        });

        auto connectSliders = [this](
            QSlider **bars, int count, auto action) {
            for (int i = 0; i < count; ++i) {
                connect(bars[i], &QSlider::valueChanged, this,
                    [this, action] {
                        if (!updating)
                            action();
                    });
            }
        };

        connectSliders(rgbS, 3, [this] {
            setRGB(sr->value(), sg->value(), sb->value());
        });

        connectSliders(cmykS, 4, [this] {
            c->setText(QString::number(sc->value()));
            m->setText(QString::number(sm->value()));
            y->setText(QString::number(sy->value()));
            k->setText(QString::number(sk->value()));
            fromCMYK();
        });

        connectSliders(hsvS, 3, [this] {
            h->setText(QString::number(sh->value()));
            s->setText(QString::number(ss->value()));
            v->setText(QString::number(sv->value()));
            fromHSV();
        });

        connect(choose, &QPushButton::clicked, this, [this] {
            QColor color = QColorDialog::getColor(Qt::white, this);

            if (color.isValid())
                setRGB(color.red(), color.green(), color.blue());
        });

        main->addStretch();

        setRGB(123, 44, 191);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Window window;
    window.show();

    return app.exec();
}
