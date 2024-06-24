//
// Created by amemiya7 on 2024/6/20.
//

#pragma once


class Gravity {
public:
    Gravity();
    Gravity(int level);
    //when press Key S enable slow drop mode
    //set level += 5
    void setSoftDrop();

    void unsetSoftDrop();

    //set level += 1
    void levelUp();

    //call whenever level changes
    void updateFallTime();

    //return seconds
    double getFallTime() const;
    int getLines() const;
    void clear();
    void setHardDrop();
    int getLevel() const;
    void setNoGravity();
    void addLines(int lines);
private:
    int level = 1;
    double fallTime = 0;
    bool isSoftDrop = false;
    int clearedLines = 0;
    bool noGravity = false;
};
