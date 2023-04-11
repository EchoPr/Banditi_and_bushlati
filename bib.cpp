#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <queue>
#include <utility>
#include <algorithm>
#include <map>

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

const int FROZEN = -1; //клетка которая не используется на поле
const int MINTEMP = 0;

const int POLICE = 0;
const int BURGLARS = 1;

const int LFREE = 0;
const int LBUSY = 1;

const int PSIZE = 4; //длина ввода
const Coords NOINFORMATION = { -1, -1 }; //нет информации о позиции

const int EMPTY = -1;
const int MOVEDTO = 0;
const int MOVEDFROM = 1;

const int MOVEUP = 1;
const int STANDBY = 0;
const int MOVEDOWN = -1;
const int MOVERIGHT = 1;
const int MOVELEFT = -1;

const int MAXHEAT = 300;
const int INF = 1000000000;

const int HEIGHT = 17;
const int WIDTH = 25;

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
            move_ = 'U';
        if (vert == MOVEDOWN)
            move_ = 'D';
        if (horiz == MOVELEFT)
            move_ = 'L';
        if (vert == MOVERIGHT)
            move_ = 'R';
        if (vert == horiz == STANDBY)
            move_ = 'S';
    }


    char get_move()
    {
        return move_;
    }


    void enter_in_lift(int steps)
    {
        steps_in_lift_left_ = steps;
        in_lift_ = true;
    }


protected:
    Coords coords_;
    bool in_lift_;
    int steps_in_lift_left_;
    char move_;
};


class Burglar : public Person //грабитель и всё, что с ним связано
{
public:
    Burglar(int x, int y)
    {
        coords_ = Coords(x, y);
        in_lift_ = false;
        caught_ = false;
        steps_in_lift_left_ = 0;
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
        steps_in_lift_left_ = 0;
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



map <pair <int, int>, vector <pair <int, int>>> adj;
vector <vector <int>> field(HEIGHT, vector <int>(WIDTH));

bool is_node(int row, int col)
{
    return row % 4 + col % 4 == 0;
}

void fill_field()
{
    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            field[row][col] = 0;

    for (int row = 1; row < HEIGHT; row += 4)
        for (int col = 1; col < WIDTH; col += 4)
            for (int r = row; r < row + 3; r++)
                for (int c = col; c < col + 3; c++)
                    field[r][c] = 1;

    for (int i = 1; i < HEIGHT; i += 4)
        for (int j = i; j < i + 3; j++)
            field[j][0] = field[j][12] = field[j][24] = 1;
}

void get_neighbours(int row, int col)
{
    pair<int, int> left = { -1, -1 };
    pair<int, int> right = { -1, -1 };
    pair<int, int> top = { -1, -1 };
    pair<int, int> bottom = { -1, -1 };

    for (int l = col - 1; l >= 0; l--)
        if (field[row][l] == 0 && is_node(row, l))
        {
            left = { row, l };
            break;
        }

    for (int r = col + 1; r < WIDTH; r++)
        if (field[row][r] == 0 && is_node(row, r))
        {
            right = { row, r };
            break;
        }

    for (int t = row - 1; t >= 0; t--)
        if (field[t][col] == 0 && is_node(t, col))
        {
            top = { t, col };
            break;
        }

    for (int b = row + 1; b < HEIGHT; b++)
        if (field[b][col] == 0 && is_node(b, col))
        {
            bottom = { b, col };
            break;
        }

    adj[{row, col}].push_back(left);
    adj[{row, col}].push_back(right);
    adj[{row, col}].push_back(top);
    adj[{row, col}].push_back(bottom);
}

void fill_adj()
{
    fill_field();

    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            if (field[row][col] == 0)
                get_neighbours(row, col);
}


class Node
{
public:
    Node()
    {
        left_dist_ = top_dist_ = right_dist_ = bottom_dist_ = INF;

        left_seg_direction_.assign(5, EMPTY);
        top_seg_direction_.assign(5, EMPTY);
        right_seg_direction_.assign(5, EMPTY);
        bottom_seg_direction_.assign(5, EMPTY);

        left_seg_direction_.assign(5, EMPTY);
        top_seg_direction_.assign(5, EMPTY);
        right_seg_direction_.assign(5, EMPTY);
        bottom_seg_direction_.assign(5, EMPTY);

    }

    void set_coords(int row, int col)
    {
        row_ = row;
        col_ = col;
    }

    vector <int> get_neighbours_info()
    {
        //здесь Егор
    }

    void seg_update()
    {
        for (int i = 0; i < 5; ++i)
            if (left_seg_direction_[i] != EMPTY)
            {
                //ЛЕВЫЙ ОТРЕЗОК
                char move = left_seg_guy_[i].get_move();

                // Шарам-барам!
                if (i == 0 && (move == 'R' || move == 'U' || move == 'D'))
                    left_seg_direction_[i] = EMPTY;
                if (i == 4 && (move == 'L' || move == 'U' || move == 'D'))
                    left_seg_direction_[i] = EMPTY;

                if (move == 'R')
                {
                    left_seg_direction_[i] = EMPTY;
                    left_seg_direction_[i - 1] = MOVEDTO;
                }
                if (move == 'L')
                {
                    left_seg_direction_[i] = EMPTY;
                    left_seg_direction_[i + 1] = MOVEDFROM;
                }
            }

        for (int i = 0; i < 5; ++i)
            if (right_seg_direction_[i] != EMPTY)
            {
                //ПРАВЫЙ ОТРЕЗОК
                char move = right_seg_guy_[i].get_move();

                // Шарам-барам!
                if (i == 0 && (move == 'L' || move == 'U' || move == 'D'))
                    right_seg_direction_[i] = EMPTY;
                if (i == 4 && (move == 'R' || move == 'U' || move == 'D'))
                    right_seg_direction_[i] = EMPTY;

                if (move == 'R')
                {
                    right_seg_direction_[i] = EMPTY;
                    right_seg_direction_[i + 1] = MOVEDFROM;
                }
                if (move == 'L')
                {
                    right_seg_direction_[i] = EMPTY;
                    right_seg_direction_[i - 1] = MOVEDTO;
                }
            }

        for (int i = 0; i < 5; ++i)
            if (top_seg_direction_[i] != EMPTY)
            {
                //ВЕРХНИЙ ОТРЕЗОК
                char move = top_seg_guy_[i].get_move();

                // Шарам-барам!
                if (i == 0 && (move == 'R' || move == 'L' || move == 'D'))
                    top_seg_direction_[i] = EMPTY;
                if (i == 4 && (move == 'R' || move == 'L' || move == 'U'))
                    top_seg_direction_[i] = EMPTY;

                if (move == 'U')
                {
                    top_seg_direction_[i] = EMPTY;
                    top_seg_direction_[i + 1] = MOVEDFROM;
                }
                if (move == 'D')
                {
                    top_seg_direction_[i] = EMPTY;
                    top_seg_direction_[i - 1] = MOVEDTO;
                }
            }

        for (int i = 0; i < 5; ++i)
            if (bottom_seg_direction_[i] != EMPTY)
            {
                //НИЖНИЙ ОТРЕЗОК
                char move = bottom_seg_guy_[i].get_move();

                // Шарам-барам!
                if (i == 0 && (move == 'R' || move == 'L' || move == 'U'))
                    bottom_seg_direction_[i] = EMPTY;
                if (i == 4 && (move == 'R' || move == 'L' || move == 'D'))
                    bottom_seg_direction_[i] = EMPTY;

                if (move == 'U')
                {
                    bottom_seg_direction_[i] = EMPTY;
                    bottom_seg_direction_[i - 1] = MOVEDTO;
                }
                if (move == 'D')
                {
                    bottom_seg_direction_[i] = EMPTY;
                    bottom_seg_direction_[i + 1] = MOVEDFROM;
                }
            }

    }
    //Витёк Кабэйнин должен написать метод обновления отрезка

protected:
    vector <int> left_seg_direction_, right_seg_direction_, top_seg_direction_, bottom_seg_direction_;
    vector <Policeman> left_seg_guy_, right_seg_guy_, top_seg_guy_, bottom_seg_guy_;
    int left_dist_, top_dist_, right_dist_, bottom_dist_;
    int row_, col_;
};

vector <Node> nodes;
void set_nodes()
{
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
        {
            Node nd;
            nd.set_coords(i, j);
            if (is_node(i, j)) nodes.push_back(nd);
        }
}

void update_nodes()
{
    for (Node x : nodes) x.seg_update();
}


class LiftNode : public Node
{
public:
    LiftNode()
    {

    }

protected:
    vector <int> floor1_, floor2_, floor3_, floor4_, floor5_;
};




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


void make_move_burglar() //МОЛИТЕСЬ ТУТ БУДЕТ МНОГО БАГОВ. NO MORE.. upd: ARE YOU SURE ABOUT THAT?
{
    for (auto x : burglars)
    {
        if (x.is_caught())
            continue;

        Coords burglar = x.get_coords();
        int row = burglar.row;
        int col = burglar.col;

        auto change_policemen_position = get_position_difference(row, col); // рассчёт направления движения полицейского


    }
}

int get_manhattan_dist(pair<int, int> A, pair<int, int> B)
{
    return abs(A.first - B.first) + abs(A.second - B.second);
}

void reset_last_stage_policemen() //обновить координаты полицейских
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





