#pragma once

#include "panels/tPanel.h"

class tPanPetrel : public tPanel {
    //Q_OBJECT
public:
    static const int PaddingX = 8; // Left side only right will be 4
    static const int SpacingX = 10; // Between elements

    static const int W_1_1 = 382; // 1 item per row
    static const int W_1_2 = 185; // 2 items per row
    static const int W_1_3 = 120; // 3 items per row
    static const int W_2_3 = W_1_3 + 1 * (W_1_3 + SpacingX); // 4 items per row

    static const int W_1_4 = 88; // 4 items per row
    static const int W_2_4 = W_1_4 + 1 * (W_1_4 + SpacingX); // 4 items per row
    static const int W_3_4 = W_1_4 + 2 * (W_1_4 + SpacingX); // 4 items per row

    static const int W_1_5 = 69; // 5 items per row
    static const int W_2_5 = W_1_5 + 1 * (W_1_5 + SpacingX); // 5 items per row
    static const int W_3_5 = W_1_5 + 2 * (W_1_5 + SpacingX) - 1; // 5 items per row
    static const int W_4_5 = W_1_5 + 3 * (W_1_5 + SpacingX); // 5 items per row

    static const int W_1_6 = 55; // 6 items per row
    static const int W_2_6 = W_1_6 + 1 * (W_1_6 + SpacingX); // 6 items per row
    static const int W_3_6 = W_1_6 + 2 * (W_1_6 + SpacingX); // 6 items per row
    static const int W_4_6 = W_1_6 + 3 * (W_1_6 + SpacingX); // 6 items per row
    static const int W_5_6 = W_1_6 + 4 * (W_1_6 + SpacingX); // 6 items per row

    static const int W2plus = 210; // 2 items per row
    static const int W2minus = W_1_1 - W2plus - SpacingX; // 2 items per row

    static const int WSB_2_0 = 60; // SpinBox 88
    static const int WSB_2_1 = 80; // SpinBox 88.8

    // Equicolumn layout
    static const int X_1_1 = PaddingX + (W_1_1 + SpacingX) * 0;

    static const int X_1_2 = PaddingX + (W_1_2 + SpacingX) * 0;
    static const int X_2_2 = PaddingX + (W_1_2 + SpacingX) * 1;

    static const int X_1_3 = PaddingX + (W_1_3 + SpacingX) * 0;
    static const int X_2_3 = PaddingX + (W_1_3 + SpacingX) * 1;
    static const int X_3_3 = PaddingX + (W_1_3 + SpacingX) * 2;

    static const int X_1_4 = PaddingX + (W_1_4 + SpacingX) * 0;
    static const int X_2_4 = PaddingX + (W_1_4 + SpacingX) * 1;
    static const int X_3_4 = PaddingX + (W_1_4 + SpacingX) * 2;
    static const int X_4_4 = PaddingX + (W_1_4 + SpacingX) * 3;

    static const int X_1_5 = PaddingX + (W_1_5 + SpacingX) * 0;
    static const int X_2_5 = PaddingX + (W_1_5 + SpacingX) * 1;
    static const int X_3_5 = PaddingX + (W_1_5 + SpacingX) * 2;
    static const int X_4_5 = PaddingX + (W_1_5 + SpacingX) * 3;
    static const int X_5_5 = PaddingX + (W_1_5 + SpacingX) * 4;

    static const int X_1_6 = PaddingX + (W_1_6 + SpacingX) * 0;
    static const int X_2_6 = PaddingX + (W_1_6 + SpacingX) * 1;
    static const int X_3_6 = PaddingX + (W_1_6 + SpacingX) * 2;
    static const int X_4_6 = PaddingX + (W_1_6 + SpacingX) * 3;
    static const int X_5_6 = PaddingX + (W_1_6 + SpacingX) * 4;
    static const int X_6_6 = PaddingX + (W_1_6 + SpacingX) * 5;

    // Two columns: plus + minus
    static const int X_1_2_PM = PaddingX;
    static const int X_2_2_PM = PaddingX + W2plus + SpacingX;

    // Two columns: minus + plus
    static const int X_1_2_MP = PaddingX;
    static const int X_2_2_MP = PaddingX + W2minus + SpacingX;

    static const int PaddingY = 5;
    static const int SpacingY = 3;
    static const int SepSpacingY = 3 * SpacingY; // Spacing after separator widget
    static const int StdH = 35; // Standard height
    static const int FullH = StdH + SpacingY; // LF
    static const int BtnH = 30;// StdH; // Button
    static const int LabH = 20; // Label
    static const int BoxH = 30; // Boxes
    static const int SbH = BoxH; // SpinBox
    static const int CbH = 25; // CheckBox
    static const int LeH = 25; // LineEdit
    static const int EbH = BoxH; // ???
    static const int SepH = LabH; // Label

public:
    tPanPetrel(QWidget* parent, int id, QString caption = "") : tPanel(parent, id, 200, caption) {};

};
