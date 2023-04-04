#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <utility>
#include <algorithm>

#define l_status int

using namespace std;


struct Coords //структура для координат микрочелов
{
    int row, col;

    Coords() { row = -1; col = -1; }
    Coords(int r, int c) { row = r; col = c; }

    bool operator==(const Coords& right)
    {
        return (this->row == right.row && this->col == right.col);
    }

    bool operator!=(const Coords& right)
    {
        return !(*this == right);
    }

    Coords operator=(Coords op)
    {
        row = op.row;
        col = op.col;

        return *this;
    }
};

const int FROZEN = -1;
const int MINTEMP = 0;

const int POLICE = 0;
const int BURGLARS = 1;

const int LFREE = 0;
const int LBUSY = 1;

const int PSIZE = 4; //длина ввода
const Coords NOINFORMATION = { -1, -1 }; //нет информации о позиции

const int MOVEDTO = 0;
const int MOVEDFROM = 1;

const int MOVEUP = 1;
const int STANDBY = 0;
const int MOVEDOWN = -1;
const int MOVERIGHT = 1;
const int MOVELEFT = -1;

const int MAXHEAT = 300;


class Person // класс который используется как шаблон для классов микрочелов
{
public:
    Coords get_coords()
    {
        return coords_;
    }

    void set_coords(Coords crd)
    {
        coords_ = crd;
    }

    void set_move(int vert, int horiz)
    {
        if (vert == MOVEUP)
            move = 'U';
        if (vert == MOVEDOWN)
            move = 'D';
        if (horiz == MOVELEFT)
            move = 'L';
        if (vert == MOVERIGHT)
            move = 'R';
        if (vert == horiz == STANDBY)
            move = 'S';
    }

protected:
    Coords coords_;
    bool in_lift_;
    int steps_in_lift_left;
    char move;
};


class Burglar : public Person //грабитель и всё, что с ним связано
{
public:
    Burglar(int x, int y)
    {
        coords_ = Coords(x, y);
        in_lift_ = false;
        caught_ = false;
        steps_in_lift_left = 0;
    }

    void imprison()
    {
        caught_ = true;
    }

    bool is_caught()
    {
        return caught_;
    }

private:
    bool caught_;
};


class Policeman : public Person //мусор (лол) и всё, что с ним связано
{
public:
    Policeman(int x, int y)
    {
        coords_ = Coords(x, y);
        in_lift_ = false;
        steps_in_lift_left = 0;
    }
};


vector <Burglar> burglars(4, Burglar(-1, -1));
vector <Policeman> policemen(4, Policeman(-1, -1));
class FieldSituation //класс отвечающий за поле и события на нём
{
public:
    l_status l_lift, m_lift, r_lift;
    int burglars_not_caught;

    FieldSituation()
    {
        burglars_not_caught = 4;
        l_lift = m_lift = r_lift = LFREE;
    }

    void set_lift_status(string lifts_status)
    {
        l_lift = lifts_status[0] == 'F' ? LFREE : LBUSY;
        m_lift = lifts_status[1] == 'F' ? LFREE : LBUSY;
        r_lift = lifts_status[2] == 'F' ? LFREE : LBUSY;
    }

    void check_collisions()
    {
        for (auto b : burglars)
            for (auto p : policemen)
            {
                if (b.get_coords() == p.get_coords())
                {
                    b.imprison();
                    burglars_not_caught--;
                }
            }
    }
};
FieldSituation field_situation;


vector <Policeman> policemen_from_last_stage(4, Policeman(-1, -1)); //положение полисменов на предыдущем ходе


class HeatMap
{
public:
    HeatMap()
    {
        heat_map_.assign(17, vector <int>(25, 0));

        for (int i = 1; i < 16; i += 4)
            for (int j = 1; j < 24; j += 4)
            {
                heat_map_[i][j] = FROZEN;
                heat_map_[i][j + 1] = FROZEN;
                heat_map_[i][j + 2] = FROZEN;

                heat_map_[i + 1][j] = FROZEN;
                heat_map_[i + 1][j + 1] = FROZEN;
                heat_map_[i + 1][j + 2] = FROZEN;

                heat_map_[i + 2][j] = FROZEN;
                heat_map_[i + 2][j + 1] = FROZEN;
                heat_map_[i + 2][j + 2] = FROZEN;
            }
    }

    void warm_up(Coords crd)
    {
        queue <pair <Coords, int>> deq; //pair <координаты, температура>
        deq.push({ crd, MAXHEAT });

        vector <vector <int>> visited(17);
        for (int i = 0; i < 17; i++)
            visited[i].assign(25, 0);

        while (!deq.empty())
        {
            pair <Coords, int> step = deq.front();
            deq.pop();
            int row = step.first.row;
            int col = step.first.col;
            int heat = step.second;

            visited[row][col] = 1;
            heat_map_[row][col] += heat;

            for (int x = -1; x <= 1; ++x)
                for (int y = -1; y <= 1; ++y)
                {
                    int r = row + x;
                    int c = col + y;
                    int cnt = 0;
                    if (x == 0)
                        ++cnt;
                    if (y == 0)
                        ++cnt;

                    if (r >= 0 && r < 17 && c >= 0 && c < 25 && cnt != 2 && cnt != 0 && heat_map_[r][c] != FROZEN && heat_map_[r][c] != MINTEMP && visited[r][c] == 0)
                        deq.push({ {r, c}, heat - 1 });
                }
        }
    }

    void cool()
    {
        for (int i = 0; i < 17; i++)
            for (int j = 0; j < 25; j++)
            {
                if (heat_map_[i][j] != FROZEN)
                    heat_map_[i][j] = max(heat_map_[i][j] - 1, MINTEMP);
            }

    }


private:
    // Массив 17х25
    vector <vector <int>> heat_map_; //vindetta 

};


HeatMap heat_map;

int player;
int turnes_left;

string get_input_data();
void print_turn(string turn);
void self_init();
bool game_stop();
void make_move_burglar();
void make_move_police();
void input_processor(string turn);
void reset_last_stage_policemen();
vector <int> get_position_difference(int row, int col);
int get_coord_difference(int c1, int c2);
int get_manhattan_dist(pair<int, int> A, pair<int, int> B);





int main()
{
    cin >> turnes_left >> player;

    self_init();

    while (!game_stop())
    {
        input_processor(get_input_data());

        field_situation.check_collisions(); // Были пойманы бандиты

        // Ладно, за работу
        if (player == BURGLARS)
        {
            make_move_burglar();
        }
        else
        {
            for (auto x : burglars)
            {
                Coords crd = x.get_coords();
                if (crd != NOINFORMATION)
                    heat_map.warm_up(crd);
            }

            make_move_police();
            heat_map.cool();
        }

        print_turn("1");

        turnes_left--;

        reset_last_stage_policemen();
    }
}






Coords translate(string pos) //переводит ход из внешнего формата во внутренний
{
    return (pos != "??" ? Coords(pos[1] - 'A', pos[0] - 'A') : Coords(-1, -1));
    //return (pos != "??" ? {pos[1] - 'A', pos[0] - 'A'} : NOINFORMATION);
}

// ?? ?? ?? ?? FFF 
// AJ AM AM AP FFF
// Agent Hitler FBI
void input_processor(string turn) //ЧВК П.А.Д.Л.А. (обработчик ввода (распихивает всё по классам))
{
    vector <Coords> positions;

    stringstream ss(turn);
    for (int i = 0; i < PSIZE; ++i) {
        string pos;
        ss >> pos;

        positions.push_back(translate(pos));
    }
    //прочитать пятую строку и использовать метод set_lift_status() (будет ещё перегрузка для него)

    string lift_status;
    ss >> lift_status;

    field_situation.set_lift_status(lift_status);

    if (player == POLICE)
    {
        for (int i = 0; i < PSIZE; ++i)
        {
            if (positions[i] != NOINFORMATION)
                burglars[i].set_coords(positions[i]);
        }
    }
    else
    {
        for (int i = 0; i < PSIZE; ++i)
        {
            policemen[i].set_coords(positions[i]);
        }
    }
}


void self_init() //инициализация в зависимости от того, за кого мы играем
{
    if (player == BURGLARS)
    {
        string conf("AA BA CA DA");
    }

    policemen[0].set_coords(translate("AK"));
    policemen[1].set_coords(translate("AL"));
    policemen[2].set_coords(translate("AN"));
    policemen[3].set_coords(translate("AO"));
}


string get_input_data()
{
    string turn;
    getline(cin, turn);

    return turn;
};


void print_turn(string turn)
{
    cout << turn << endl;
};


bool game_stop()
{
    return field_situation.burglars_not_caught == 0 || turnes_left == 0;
}


void make_move_police()
{

}


void make_move_burglar() //МОЛИТЕСЬ ТУТ БУДЕТ МНОГО БАГОВ
{
    for (auto x : burglars)
    {
        if (x.is_caught())
            continue;

        Coords burglar = x.get_coords();
        int row = burglar.row;
        int col = burglar.col;

        auto change_policemen_position = get_position_difference(row, col); // рассчёт направления движения полицейского

        //бандит не должен ОСТАВАТЬСЯ на пересечении этажа с лестницой. Проходить - да, но не оставаться.

        ////////////////
        ////СЛУЧАЙ 1////
        ////////////////
        if ((col == 'E' - 'A' || col == 'I' - 'A' || col == 'Q' - 'A' || col == 'U' - 'A') && (row != 'E' - 'A' && row != 'I' - 'A' && row != 'M' - 'A')) //бандит на лестнице
        {
            vector <int> hunting_up; //полицейский, охотящийся за нами на этаже выше (индексы полицейских)
            vector <int> hunting_down; //полицейский, охотящийся за нами на этаже ниже (индексы полицейских)

            for (int ind = 0; ind < PSIZE; ind++)
                if (change_policemen_position[ind] == MOVEDTO) //полицейский охотится за нами
                {
                    Coords police_guy_c = policemen[ind].get_coords();
                    int r = police_guy_c.row;
                    int c = police_guy_c.col;

                    //те, что на этаже
                    if (r == 'Q' - 'A' || r == 'M' - 'A' || r == 'I' - 'A' || r == 'E' - 'A' || r == 'A' - 'A') //полисмен на этаже
                    {
                        if (get_coord_difference(row, r) <= 4 && r > row) //а именно на этаже выше
                            hunting_up.push_back(ind);
                        else if (get_coord_difference(row, r) <= 4 && r < row) //а именно на этаже ниже
                            hunting_down.push_back(ind);
                    }

                    //те, что на той же вертикали
                    if (c == col)
                    {
                        if (r > row) //те, что над нами
                            hunting_up.push_back(ind);
                        else if (r < row)
                            hunting_down.push_back(ind);
                    }
                }


            //вектора полицейских, охотящихся за нами, заполнены

            if (hunting_up.size() == 0 || hunting_down.size() == 0)
            {
                x.set_move(STANDBY, STANDBY);
                continue;
            }

            int good_up = 1; //чист ли верх НАД нами. Можно ли туда идти.
            for (int ind : hunting_up)
            {
                auto x = policemen[ind];
                Coords coords = x.get_coords();
                int r = coords.row;
                int c = coords.col;

                int dist = get_manhattan_dist({ row, r }, { col, c });

                if (dist < 5)//исправлено с dist < 4
                    good_up = 0;
            }

            int good_down = 1; //чист ли верх ПОД нами. Можно ли туда идти.
            for (int ind : hunting_down)
            {
                auto x = policemen[ind];
                Coords coords = x.get_coords();
                int r = coords.row;
                int c = coords.col;

                int dist = get_manhattan_dist({ row, r }, { col, c });

                if (dist < 5)
                    good_down = 0;
            }

            /*
            if (good_up == good_down)
            {
                 //оценка верхней и нижней позиций. Идем туда, где лучше
            }
            else*/ if (good_up == 1)
            {
                int floor = row;
                while (!(floor == 'E' - 'A' || floor == 'I' - 'A' || floor == 'M' - 'A' || floor == 'Q' - 'A'))
                    floor++;

                int sides[4] = { 0 };	//left, right, top, bottom
                int left = 1, right = 1, top = 1, bottom = 1;

                for (int ind : hunting_up)
                {
                    Coords C = policemen[ind].get_coords();
                    int r = C.row;
                    int c = C.col;

                    if (r == floor && c < col)
                        left = 0;
                    else if (r == floor && c > col)
                        right = 0;
                    else if (c == col && r > floor)
                        top = 0;
                    else if (c == col && r < floor)
                        bottom = 0;
                }

                //if (left == 1)
                    //целевая точка = (floor, col - 1);
                //else if (right == 1)
                    //целевая точка = (floor, col + 1);
                //else if (top == 1)
                    //целевая точка = (floor + 1, col);
                //else if (bottom = 1)
                    //целевая точка = (floor - 1, col);

            }
            else if (good_down == 1)
            {
                int floor = row;
                while (!(floor == 'E' - 'A' || floor == 'I' - 'A' || floor == 'M' - 'A' || floor == 'Q' - 'A'))
                    floor--;

                int left = 1, right = 1, top = 1, bottom = 1;

                for (int ind : hunting_down)
                {
                    Coords C = policemen[ind].get_coords();
                    int r = C.row;
                    int c = C.col;

                    if (r == floor && c < col)
                        left = 0;
                    else if (r == floor && c > col)
                        right = 0;
                    else if (c == col && r > floor)
                        top = 0;
                    else if (c == col && r < floor)
                        bottom = 0;
                }

                //if (left == 1)
                    //целевая точка = (floor, col - 1);
                //else if (right == 1)
                    //целевая точка = (floor, col + 1);
                //else if (top == 1)
                    //целевая точка = (floor + 1, col);
                //else if (bottom = 1)
                    //целевая точка = (floor - 1, col);
            }
            else
            {
                x.set_move(STANDBY, STANDBY);
            }
        }

        ////////////////
        ////СЛУЧАЙ 2////
        ////////////////
        /*if ()  //бандит на этаже
        {

        }
        */
        /*
        осталось посмотреть на то, что полисмены стоят по разные стороны от нас и движутся в нашу сторону.
        если это так, то смотрим возможности отхода.
        По стратегии смотрим на расстояния от нас до полисменов и оцениваем расстояние, куда мы можем сместиться. Идем туда.
        С учетом того, что если мы можем идти куда-то(место 1) и (в угол | в ситуацию 3) - идем в место 1.
        */
    }
}

int get_manhattan_dist(pair<int, int> A, pair<int, int> B)
{
    return abs(A.first - B.first) + abs(A.second - B.second);
}

void reset_last_stage_policemen()
{
    //изменить положение полисменов с предыдущего шага
    for (int i = 0; i < policemen.size(); i++)
        policemen_from_last_stage[i] = policemen[i];
}

vector <int> get_position_difference(int row, int col)
{
    vector <int> res(PSIZE);

    for (int i = 0; i < PSIZE; i++)
    {
        Coords old = policemen_from_last_stage[i].get_coords();
        int r_old = old.row;
        int c_old = old.col;

        Coords cur = policemen[i].get_coords();
        int r_cur = cur.row;
        int c_cur = cur.col;

        int hor_diff = get_coord_difference(row, r_old) - get_coord_difference(row, r_cur);
        int col_diff = get_coord_difference(col, c_old) - get_coord_difference(col, c_cur);

        if (hor_diff == 1)
            res[i] = MOVEDTO;
        else if (hor_diff == -1)
            res[i] = MOVEDFROM;

        if (col_diff == 1)
            res[i] = MOVEDTO;
        else if (col_diff == -1)
            res[i] = MOVEDFROM;
    }

    return res;
}

int get_coord_difference(int c1, int c2)
{
    return abs(c1 - c2);
}

/*



 */





