#include "Memory.h"
#include "Mathem.h"
#include "DrawTools.h"
#include "thread"


int main()
{
    //DWORD - 4 байта, хранит в 10-системе; void* или LPCVOID - указатель, хранит в 16-системе, имеет любой размер. Можно из DWORD --> LPCVOID (void*) или LPCVOID (void*) --> DWORD
    setlocale(LC_ALL, "");
    const WCHAR* procName; //WCHAR* procName;
    //cout << "Введите имя процесса: " << endl;
    //std::wcin >> procName;
    procName = L"hl2.exe";
    Memory mem = Memory{procName};
    std::thread myThread(startRender);

    LPCVOID mainPlayerAddr = (LPCVOID) mem.ReadAddr<DWORD>((DWORD)mem.ClientModuleAddr + 0x004C88E8); //Получение указателя на структуру нашего игрока.
    Player mainPlayer = Player(Vec3(mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x0), mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x4), mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x8) ), 
                        mem.ReadAddr<int>((DWORD)mainPlayerAddr + 0x9C), 
                        mem.ReadAddr<int>((DWORD)mainPlayerAddr + 0x94)
    );
    mainPlayer.axis_XY = mem.ReadAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x0); //(DWORD в памяти, но если взять байты и во float переменную считать, то получим float значение (Как и нужно)) - UPD: это уже не правильно, так делали из-за того, что CE не правильно читал структуру
    mainPlayer.axis_XZ = mem.ReadAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x4); //float
    
    int numPlayers = mem.ReadAddr<int>((DWORD)mem.EngineModuleAddr + 0x5EC82C); //engine.dll + m_iNumPlayers --> Кол-во игроков на карте
    Player entity[32]{};

    vMatrix matrix;
    matrix.read_vMatrix(mem, (DWORD)mem.EngineModuleAddr + 0x5B0D68);

    while (true) {
        mainPlayer.vec.X = mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x0); mainPlayer.vec.Y = mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x4); mainPlayer.vec.Z = mem.ReadAddr<float>((DWORD)mainPlayerAddr + 0x260 + 0x8);
        mainPlayer.teamNum = mem.ReadAddr<int>((DWORD)mainPlayerAddr + 0x9C);
        mainPlayer.health = mem.ReadAddr<int>((DWORD)mainPlayerAddr + 0x94);
        mainPlayer.axis_XY = mem.ReadAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x0);
        mainPlayer.axis_XZ = mem.ReadAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x4);

        for (int i{}; i < numPlayers; i++) {
            LPCVOID entityAddr = (LPCVOID)mem.ReadAddr<DWORD>((DWORD)mem.ClientModuleAddr + 0x4D5AE4 + 0x10 * i); //Получаем указатель на структуру игрока.
            entity[i] = Player(
                Vec3(mem.ReadAddr<float>((DWORD)entityAddr + 0x260 + 0x0), mem.ReadAddr<float>((DWORD)entityAddr + 0x260 + 0x4), mem.ReadAddr<float>((DWORD)entityAddr + 0x260 + 0x8)), //Вектора сущностей(Координаты)
                mem.ReadAddr<DWORD>((DWORD)entityAddr + 0x9C), //m_iTeamNum
                mem.ReadAddr<DWORD>((DWORD)entityAddr + 0x94)  //m_iHealth
            );

        }

        //Aim
        if (GetAsyncKeyState(VK_LBUTTON))
        {
            Player crntTarget;
            float minDistance = 99999.0f;
            float distance{};
            bool  hasOppositeTeam_status = false;

            for (auto& ent : entity) {
                if (ent.teamNum != mainPlayer.teamNum && ent.health >= 2) {
                    hasOppositeTeam_status = true;
                    distance = sqrt(
                        (ent.vec.X - mainPlayer.vec.X) * (ent.vec.X - mainPlayer.vec.X) +
                        (ent.vec.Y - mainPlayer.vec.Y) * (ent.vec.Y - mainPlayer.vec.Y) +
                        (ent.vec.Z - mainPlayer.vec.Z) * (ent.vec.Z - mainPlayer.vec.Z)
                    );
                    if (distance < minDistance) {
                        minDistance = distance;
                        crntTarget = ent;
                    }
                }
            }
            if (hasOppositeTeam_status == true) {
                Vec2 angle = calcAngle(mainPlayer.vec, crntTarget.vec, distance);
                mainPlayer.axis_XY = angle.X;
                mainPlayer.axis_XZ = angle.Y;

                mem.WriteAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x0, mainPlayer.axis_XY);
                mem.WriteAddr<float>((DWORD)mem.EngineModuleAddr + 0x47C33C + 0x4, mainPlayer.axis_XZ);
            }
        }
        //ESP
        /*
        GetWindowRect(FindWindow(NULL, L"Counter-Strike Source"), &m_Rect);
        matrix.read_vMatrix(mem, (DWORD)mem.EngineModuleAddr + 0x5B0D68);
        for (int i{}; i < numPlayers; i++) {
            if (entity[i].teamNum == mainPlayer.teamNum) 
                continue;
            if (entity[i].health <= 2)
                continue;
            
            float entityScreenPosition[3]; //Будет хранить координаты пикселей на экране
            if (worldToScreen(matrix, entity[i].vec, entityScreenPosition, m_Rect)) {
                float distance = sqrt(
                    (entity[i].vec.X - mainPlayer.vec.X) * (entity[i].vec.X - mainPlayer.vec.X) +
                    (entity[i].vec.Y - mainPlayer.vec.Y) * (entity[i].vec.Y - mainPlayer.vec.Y) +
                    (entity[i].vec.Z - mainPlayer.vec.Z) * (entity[i].vec.Z - mainPlayer.vec.Z)
                );
                //DrawESP(entityScreenPosition[0] - m_Rect.left, entityScreenPosition[1] - m_Rect.top, distance, m_Rect);
                int width = 18100 / distance;
                int height = 36000 / distance;

                float start_x = entityScreenPosition[0] - m_Rect.left - (width / 2);
                float start_y = entityScreenPosition[1] - m_Rect.top - height;

                //ImGui::GetBackgroundDrawList()->AddRect({start_x, start_y}, {start_x + width, start_y + height}, ImColor(255,255,255)); - рисовка esp-бокса
            }
        }
        */

        
    }
} 





/*Заметки:
    1. Оффсеты на подобии  ХП, патронов, прибавляются только после получения указателя на объект:
       1.1 Возьмем например ХП бота.
       1.2 Адрес начала списка = client.dll + offset_dwEntityList
       1.3 Возьмем из этого списка элемент 3 (третьего бота возьмем, а не первых двух).
       1.4 Для этого нужно: client.dll + offset_dwEntityList + offset_nextBotInList * i(порядковый номер элемента в списке).
       1.5 Обращаясь к памяти по этому адресу, получаем указатель(адрес) на структуру бота.
       1.6 Далее, для обращения к членам структуры таким как ХП и патроны бота, необходимо к полученному адресу прибавить offset_HP.
       1.7 Получаем ХП третьего бота.
       1.8 То есть при client.dll + offset_dwEntityList + offset_nextBotInList * i + offset_HP - мы не получим ХП нашего бота, т.к нужно после получения адреса списка, нужно обратиться по 
       этому адресу для получения указателя на сущность, то есть где хранится сам бот и от туда уже отталкиваться для получения ХП.
    2. 
       //cout << mem.ReadAddr(mem.ReadAddr((DWORD)mem.ClientModuleAddr + (DWORD)0x4C88E8) + (DWORD)0x94) << endl; Получаем адрес модуля client.dll, затем прибавляя к нему оффсет localPlayer,
       //получаем указатель на структуру игрока (в ячейке по адресу client.dll + localPlayer хранится адрес или же указатель)
       //затем складывая указатель (адрес, который хранился в ячейке и сейчас хранится в указателе) + m_iHealth (оффсет здоровья) получаем текущее ХП.
    3.
       //engine.dll + 0x47C33C + 0x0 - XY (DWORD)
       //engine.dll + 0x47C33C + 0x4 - XZ (float)
       //Разница между координатой Z в игре и памяти - 64, то есть значение координаты Z в памяти равно координата Z в игре - 64
    4. 
       Cheat Engine может не так считывать структуры в памяти, в результате чего на месте адреса хранящего координату X, например, может храниться указатель, который не имеет никакой ценности.
       Для решения проблемы связанной с отображением неправильных значений хранящихся по нужному адресу или расположением нужных значений не там где они должны быть, необходимо
       заставить Cheat Engine перечетать указанную память, чтобы он отобразил правильную структуру.
       Также, может быть так, что Cheat Engine показывает не правильный тип данных какой-либо ячейки. Например: в ячейки хранится DWORD тип равный 4 байтам, но по этому адресу, в ячейке хранится
       значение, которое имеет тип float равное 4 байтам. Cheat Engine показывает тип DWORD и значение 362176312636128, что при считывании в переменную float получим 1.23232e+2, 
       хотя там хранится 4 байт по этому адресу и при считывании этих байт в переменную float получим 36.000312030210 (Нужный нам результат). Значение скорее всего неверно из-за того, что
       Cheat Engine решил что значение является DWORD и после этого происходит отсечка бит, а не байтов(Происходит потеря бит или данных). <--- Это из-за неправильного чтения структур Cheat Engine'ом
    5.
       Аим работает, но проблема возникает, что после начала раунда аимбот не сразу начинает целиться, а только через ~ секунд 10-15. Проблема не в логике работы чита, а скорее всего в памяти, что не сразу
       данные хранящиеся там обновляются, а только через данный промежуток времени. Также, может возникнуть такая проблема, даже если все противники были уничтожены, в этом случае камеру просто разворачивает в небо
       , либо в землю, и поворачивать персонажем нельзя. Данный описанный баг выше никак не связан с логикой кода, так как производились изменения кода, и было все бессмысленно.
    6. Нужно обновить чит, чтобы можно было целиться по врагу, который находится в присяде, можно сделать через флаг состояния, и уже отталкиваться от этого.
    7. Аимбот стреляет только по координатам головы врага, он не учитывает в какую сторону повернут враг, в итоге аимбот стреляет по координатам головы, но из-за того, что голова повернута в другую сторону,
    она по сути смещена немного со своих координат, в результате чего производится промах. То есть нужно учесть вот этот поворот, чтобы вычислить смещение от координат головы.
    8. Насчет ESP-Cheat:
       1. Псмотреть как в ImGui создаватьть прозрачные окна (менюшки) на весь экран.
       2. Добавлять на это окно линии и боксы противника.
       3. Окно будет поверх окна игры, в результате получим линии которые будут вести к противнику и боксы, которые покажут где враг.
    9. Для отрисовки esp-боксов, нужно чтобы игра была в оконном режиме.
*/