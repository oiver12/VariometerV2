#pragma once

#include "main.h"

void startRender();
void updateRender();
void startUpArrow();
void UpArrowReleased();
void startDownArrow();
void DownArrowReleased();
void startEnterKey();
void EnterKeyReleased();
void startMenuKey();
void MenuKeyReleased();
void PKeyPressed();
void startRenderTimeProgress();
void updateRenderTimeProgress(float percentage, size_t vario_state_Simulation_size, vario_state_Simulation* bufferSimulation);
void startHeightDiagramm(size_t vario_state_Simulation_size, vario_state_Simulation* bufferSimulation);