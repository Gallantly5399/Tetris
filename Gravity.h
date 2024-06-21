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

    //call after setSoftDrop and setHardDrop
    //reset the level
    void reset();

    //set level += 1
    void levelUp();

    //call whenever level changes
    void updateFallTime();

    //return seconds
    double getFallTime() const;
private:
    int preLevel = 1;
    int level = 1;
    double fallTime = 0;
};
