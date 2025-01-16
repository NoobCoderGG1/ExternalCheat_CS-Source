#pragma once
#include "Memory.h"

struct vMatrix {
    float matrix[16];

    vMatrix() {}

    void read_vMatrix(Memory mem, DWORD addr) {
        for (int i{}; i < 16; i++) {
            matrix[i] = mem.ReadAddr<float>(addr + i * 4);
        }
    }

    float operator[](int index) {
        return matrix[index];
    }

};

struct Vec4 {
    float X{}, Y{}, Z{}, W{};
    Vec4() {
    }

    Vec4(float x, float y, float z, float w) : X{ x }, Y{ y }, Z{ z }, W{ w } {

    }
};

struct Vec3 {
    float X{}, Y{}, Z{};
    Vec3() {
    }

    Vec3(float x, float y, float z) : X{ x }, Y{ y }, Z{ z } {
    }
};

struct Vec2 {
    float X{}, Y{};
    Vec2() {

    }
    Vec2(float x, float y) {
        this->X = x;
        this->Y = y;
    }
};

Vec2 calcAngle(Vec3 src, Vec3 to, float distance) {
    float yaw{}, pitch{};
    Vec3 vec_dir = Vec3(to.X - src.X, to.Y - src.Y, to.Z - src.Z); //¬ектор направлени€

    yaw = (float)(atan2(vec_dir.Y, vec_dir.X) * 180.0f / M_PI); //atan2 возвращает в радианах, стоит учитывать, поэтому умножаем на 180/Pi
    pitch = -(float)(atan2(vec_dir.Z, distance) * 180.0f / M_PI);

    return Vec2(pitch, yaw);
}

struct Player {
    Vec3 vec;
    int health{}, teamNum{};
    float axis_XY{}, axis_XZ{};

    Player() {

    }

    Player(Vec3 vector, int m_teamNum, int m_iHealth) : vec{ vector } {
        teamNum = m_teamNum;
        health = m_iHealth;
    }
};

