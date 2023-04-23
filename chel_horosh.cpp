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
#include <time.h>

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



const int POLICE = 0;
const int BURGLARS = 1;

const int LFREE = 0;
const int LBUSY = 1;

const int PSIZE = 4; //длина ввода
const Coords NOINFORMATION = { -1, -1 }; //нет информации о позиции

const int EMPTY = -1;
const int MOVEDTO = 0;
const int MOVEDFROM = 1;
const int NOT_EXIST = -2;

const int MOVEUP = 101;
const int STANDBY = 100;
const int MOVEDOWN = -101;
const int MOVERIGHT = 111;
const int MOVELEFT = -111;

const int MAXHEAT = 300;
const int INF = 1000000000;

const int HEIGHT = 17;
const int WIDTH = 25;

const int FROZEN = -273; //клетка которая не используется на поле
const int MINTEMP = 0;


map<int, char> DIRMOVES{ {MOVEUP, 'U'}, {MOVEDOWN, 'D'}, {MOVELEFT, 'L'}, {MOVERIGHT, 'R'},
                         {STANDBY, 'S'}, {-1, '#'},
                         {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}
};


#define DEBUG




void print_turn();
void self_init();
bool game_stop();
void make_move_burglar();
void make_move_police();
void input_processor();
void update_adj();
bool is_lift(int row, int col);
bool is_our_seg(int row, int col, int nd_row, int nd_col, string type);
Coords translate(string pos);






class Person // класс который используется как шаблон для классов микрочелов
{
public:

    Coords coords_;
    bool in_lift_;
    int steps_in_lift_left_;
    char move_;
    int aim_floor_;
    int last_move;

    Coords get_coords()
    {
        return coords_;
    }

    void set_coords(Coords crd)
    {
        coords_ = crd;
    }

    void set_move(int move)
    {
        move_ = DIRMOVES[move];
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

    pair<int, int> get_lift_info()
    {
        return { aim_floor_, steps_in_lift_left_ };
    }

    void update_steps_in_lift_left()
    {
        --steps_in_lift_left_;
    }

    bool is_in_lift()
    {
        return in_lift_;
    }



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
        last_move = 0;
    }

    void imprison()
    {
        this->set_coords(Coords(-1, -1));
        caught_ = true;
    }

    bool is_caught()
    {
        return caught_;
    }

    bool caught_;


};


class Policeman : public Person //мусор (лол) и всё, что с ним связано
{
public:
    bool stupid;

    Policeman() {}

    Policeman(int x, int y)
    {
        coords_ = Coords(x, y);
        in_lift_ = false;
        steps_in_lift_left_ = 0;
        stupid = true;
        last_move = 0;
    }
};


void shift_player(int step, Burglar& burg);
void shift_player(int step, Policeman& burg);


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
        for (int i = 0; i < PSIZE; i++)
            for (int j = 0; j < PSIZE; j++)
            {
                if (burglars[i].get_coords() == policemen[j].get_coords())
                {
                    burglars[i].imprison();
                    burglars_not_caught--;
                }
            }
    }

    void update_onlifts_persons()
    {
        for (int i = 0; i < PSIZE; i++)
        {
            if (burglars[i].is_in_lift()) burglars[i].update_steps_in_lift_left();
            if (policemen[i].is_in_lift()) policemen[i].update_steps_in_lift_left();
        }
    }

};
FieldSituation field_situation;





class Node
{
public:

    vector <int> left_seg_direction_, right_seg_direction_, top_seg_direction_, bottom_seg_direction_;
    vector <Policeman> left_seg_guy_, right_seg_guy_, top_seg_guy_, bottom_seg_guy_;
    int left_dist_, top_dist_, right_dist_, bottom_dist_;
    int row_, col_;

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

        left_seg_guy_.resize(5);
        right_seg_guy_.resize(5);
        bottom_seg_guy_.resize(5);
        top_seg_guy_.resize(5);

    }

    void set_coords(int row, int col)
    {
        row_ = row;
        col_ = col;
    }

    //изменения отрезков чуваков
    void set_left_seg_guy(vector<Policeman> p)
    {
        left_seg_guy_ = p;
    }

    void set_right_seg_guy(vector<Policeman> p)
    {
        right_seg_guy_ = p;
    }

    void set_top_seg_guy(vector<Policeman> p)
    {
        top_seg_guy_ = p;
    }

    void set_bottom_seg_guy(vector<Policeman> p)
    {
        bottom_seg_guy_ = p;
    }

    //изменения отрезков направлений
    void set_left_seg_direction(vector<int> p)
    {
        left_seg_direction_ = p;
    }

    void set_right_seg_direction(vector<int> p)
    {
        right_seg_direction_ = p;
    }

    void set_top_seg_direction(vector<int> i)
    {
        top_seg_direction_ = i;
    }

    void set_bottom_seg_direction(vector<int> i)
    {
        bottom_seg_direction_ = i;
    }

    Coords get_coords()
    {
        return { row_, col_ };
    }

    vector <int> get_left_seg_direction()
    {
        return left_seg_direction_;
    }

    vector <int> get_bottom_seg_direction()
    {
        return bottom_seg_direction_;
    }

    vector <int> get_top_seg_direction()
    {
        return top_seg_direction_;
    }

    vector <int> get_right_seg_direction()
    {
        return right_seg_direction_;
    }


    vector <Policeman> get_left_seg_guy()
    {
        return left_seg_guy_;
    }

    vector <Policeman> get_bottom_seg_guy()
    {
        return bottom_seg_guy_;
    }

    vector <Policeman> get_top_seg_guy()
    {
        return top_seg_guy_;
    }

    vector <Policeman> get_right_seg_guy()
    {
        return right_seg_guy_;
    }

    vector <int> get_custom_seg_direction(string dir)
    {
        if (dir == "left")
            return this->get_left_seg_direction();
        else if (dir == "right")
            return this->get_right_seg_direction();
        else if (dir == "top")
            return this->get_top_seg_direction();
        else if (dir == "bottom")
            return this->get_bottom_seg_direction();
    }

    vector <Policeman> get_custom_seg_guy(string dir)
    {
        if (dir == "left")
            return this->get_left_seg_guy();
        else if (dir == "right")
            return this->get_right_seg_guy();
        else if (dir == "top")
            return this->get_top_seg_guy();
        else if (dir == "bottom")
            return this->get_bottom_seg_guy();
    }

    void set_custom_seg_guy(vector <Policeman> p, string dir)
    {
        if (dir == "left")
            this->set_left_seg_guy(p);
        else if (dir == "right")
            this->set_right_seg_guy(p);
        else if (dir == "top")
            this->set_top_seg_guy(p);
        else if (dir == "bottom")
            this->set_bottom_seg_guy(p);
    }

    void set_custom_seg_direction(vector <int> i, string dir)
    {
        if (dir == "left")
            this->set_left_seg_direction(i);
        else if (dir == "right")
            this->set_right_seg_direction(i);
        else if (dir == "top")
            this->set_top_seg_direction(i);
        else if (dir == "bottom")
            this->set_bottom_seg_direction(i);
    }

    Node operator=(Node op)
    {
        left_seg_direction_ = op.get_left_seg_direction();
        right_seg_direction_ = op.get_right_seg_direction();
        top_seg_direction_ = op.get_top_seg_direction();
        bottom_seg_direction_ = op.get_bottom_seg_direction();

        left_seg_guy_ = op.get_left_seg_guy();
        right_seg_guy_ = op.get_right_seg_guy();
        top_seg_guy_ = op.get_top_seg_guy();
        bottom_seg_guy_ = op.get_bottom_seg_guy();

        left_dist_ = op.left_dist_;
        right_dist_ = op.right_dist_;
        top_dist_ = op.top_dist_;
        bottom_dist_ = op.bottom_dist_;

        return *this;
    }
};
int get_seg_coords(int p_row, int p_col, Node node);

class LiftNode : public Node
{
public:

    vector <int> floors_;
    vector <Policeman> floors_guys_;
    int floor_;
    int time_left_;

    LiftNode(int floor)
    {
        floors_.assign(5, EMPTY);
        floor_ = floor;
    }

    void lift_update()
    {
        for (int i = 0; i < 5; ++i)
        {
            if (floors_[i] != EMPTY)
            {
                auto info = floors_guys_[i].get_lift_info();
                int direction = info.first;
                int time_left = info.second;
                char move = floors_guys_[i].get_move();

                if (direction == floor_)
                {
                    time_left_ = time_left;
                    floors_[i] = MOVEDTO;
                }
                else
                {
                    time_left_ = INF;
                    floors_[i] = EMPTY;
                }
            }
        }
    }

    int get_floor()
    {
        return floor_;
    }

    vector <int> get_floors()
    {
        return floors_;
    }

    bool is_using()
    {
        int lift = col_ / 12;

        if (lift == 0)
        {
            return field_situation.l_lift == LBUSY;
        }
        else if (lift == 1)
        {
            return field_situation.m_lift == LBUSY;
        }
        else
        {
            return field_situation.r_lift == LBUSY;
        }
    }


};


int get_side(int row, int col, Node neighbour);

map <pair <int, int>, vector <Node>> adj;
map <pair <int, int>, vector <LiftNode>> adj_lifts;
vector <vector <int>> field(HEIGHT, vector <int>(WIDTH));

bool is_node(int row, int col) //УБРАТЬ! СРОЧНО УБРАТЬ ЛИФТЫ ИЗ НОДОВ! И ИЗ СПИСКА НОДОВ ТОЖЕ!!!! 	
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
    Node left;
    Node right;
    Node top;
    Node bottom;

    LiftNode lift_left(-1);
    LiftNode lift_right(-1);
    LiftNode lift_top(-1);
    LiftNode lift_bottom(-1);

    for (int l = col - 1; l >= 0; l--)
        if (field[row][l] == 0 && is_node(row, l))
        {
            if (is_lift(row, l))
            {
                lift_left.set_coords(row, l);
                adj_lifts[{row, col}].push_back(lift_left);
                break;
            }

            left.set_coords(row, l);
            adj[{row, col}].push_back(left);
            break;
        }

    for (int r = col + 1; r < WIDTH; r++)
        if (field[row][r] == 0 && is_node(row, r))
        {
            if (is_lift(row, r))
            {
                lift_right.set_coords(row, r);
                adj_lifts[{row, col}].push_back(lift_right);
                break;
            }

            right.set_coords(row, r);
            adj[{row, col}].push_back(right);
            break;
        }

    for (int t = row - 1; t >= 0; t--)
        if (field[t][col] == 0 && is_node(t, col))
        {
            if (is_lift(t, col))
            {
                lift_bottom.set_coords(t, col);
                adj_lifts[{row, col}].push_back(lift_top);
                break;
            }

            top.set_coords(t, col);
            adj[{row, col}].push_back(top);
            break;
        }

    for (int b = row + 1; b < HEIGHT; b++)
        if (field[b][col] == 0 && is_node(b, col))
        {
            if (is_lift(b, col))
            {
                lift_left.set_coords(b, col);
                adj_lifts[{row, col}].push_back(lift_bottom);
                break;
            }

            bottom.set_coords(b, col);
            adj[{row, col}].push_back(bottom);
            break;
        }

}
vector <Node> nodes;
vector <LiftNode> lift_nodes;


void fill_adj()
{
    fill_field();

    for (int row = 0; row < HEIGHT; row++)
        for (int col = 0; col < WIDTH; col++)
            if (field[row][col] == 0)
                get_neighbours(row, col);

    for (auto& node : nodes)
    {
        Coords c = node.get_coords();
        int row = c.row;
        int col = c.col;

        if (row == HEIGHT - 1)
        {
            vector <int> v(5, NOT_EXIST);
            node.set_top_seg_direction(v);
        }
    }

    for (auto& node : nodes)
    {
        Coords c = node.get_coords();
        int row = c.row;
        int col = c.col;

        if (row == 0)
        {
            vector <int> v(5, NOT_EXIST);
            node.set_bottom_seg_direction(v);
        }
    }

    for (auto& node : nodes)
    {
        Coords c = node.get_coords();
        int row = c.row;
        int col = c.col;

        if (col == 12)
        {
            vector <int> v(5, NOT_EXIST);
            node.set_top_seg_direction(v);
            node.set_bottom_seg_direction(v);
        }
    }
}

void set_nodes()
{
    for (int i = 0; i < HEIGHT; i++)
        for (int j = 0; j < WIDTH; j++)
        {
            LiftNode lift_nd(i); lift_nd.set_coords(i, j);
            Node nd; nd.set_coords(i, j);
            if (is_lift(i, j)) { lift_nodes.push_back(lift_nd); continue; }

            if (is_node(i, j) || (j == 12 && is_lift(i, j))) nodes.push_back(nd);
        }
}

bool is_neighbour(int row, int col, Node node)
{
    for (auto n : adj[{row, col}])
    {
        if (node.get_coords() == n.get_coords())
            return true;
    }
    return false;
}


bool is_getting_to_node(int row, int col, char move)
{
    if (move == 'U')
        ++row;
    else if (move == 'D')
        --row;
    else if (move == 'L')
        --col;
    else
        ++col;

    return is_node(row, col);
}

void update_neighbour_direction_case1(Node& node, string dir)
{
    vector <int> direction = node.get_custom_seg_direction(dir);
    direction[4] = EMPTY;
    node.set_custom_seg_direction(direction, dir);
}

void update_our_direction_case1(Node& node, string dir, int f, int t, int moved)
{
    vector <int> direction = node.get_custom_seg_direction(dir);
    direction[f] = moved;
    direction[t] = EMPTY;

    vector <Policeman> guys = node.get_custom_seg_guy(dir);
    guys[1] = guys[0];

    node.set_custom_seg_direction(direction, dir);
    node.set_custom_seg_guy(guys, dir);
}

void update_direction_case2(Node& node, string dir, int seg_pos, int trans, int moved)
{
    if (seg_pos != -1)
    {
        vector <int> direction = node.get_custom_seg_direction(dir);
        direction[seg_pos + trans] = moved;
        direction[seg_pos] = EMPTY;

        vector <Policeman> guys = node.get_custom_seg_guy(dir);
        guys[seg_pos + trans] = guys[seg_pos];

        node.set_custom_seg_direction(direction, dir);
        node.set_custom_seg_guy(guys, dir);
    }
}

void update_direction_case3(Node& node, string dir, Policeman value)
{
    vector <int> direction = node.get_custom_seg_direction(dir);
    direction[4] = MOVEDTO;

    vector <Policeman> guys = node.get_custom_seg_guy(dir);
    guys[4] = value;

    node.set_custom_seg_direction(direction, dir);
    node.set_custom_seg_guy(guys, dir);

}

void seg_update(Policeman p)
{
    Coords crd = p.get_coords();
    int p_row = crd.row;
    int p_col = crd.col;
    char move = p.get_move();

    if (is_lift(p_row, p_col))
        return;
    //СЛУЧАЙ I
    if (is_node(p_row, p_col))
    {
        int to_ind = -1;

        for (int i = 0; i < nodes.size(); i++)
            if (is_neighbour(p_row, p_col, nodes[i]))
            {
                //left = 0, right = 1, top = 2, bottom = 3
                int side = get_side(p_row, p_col, nodes[i]);

                if (side == 0)
                {
                    update_neighbour_direction_case1(nodes[i], "left");

                    if (move == 'L')
                        to_ind = i;
                }
                else if (side == 1)
                {
                    update_neighbour_direction_case1(nodes[i], "right");

                    if (move == 'R')
                        to_ind = i;
                }
                else if (side == 2)
                {
                    update_neighbour_direction_case1(nodes[i], "top");

                    if (move == 'U')
                        to_ind = i;
                }
                else
                {
                    update_neighbour_direction_case1(nodes[i], "bottom");

                    if (move == 'D')
                        to_ind = i;
                }
            }

        int my_node;
        for (int i = 0; i < nodes.size(); i++)
        {
            Coords coo = nodes[i].get_coords();
            int r = coo.row;
            int c = coo.col;
            if (make_pair(r, c) == make_pair(p_row, p_col))
                my_node = i;
        }

        if (move == 'L')
        {
            update_our_direction_case1(nodes[my_node], "left", 1, 0, MOVEDFROM);
            if (to_ind != -1)
                update_our_direction_case1(nodes[to_ind], "right", 3, 4, MOVEDTO);
        }
        else if (move == 'R')
        {
            update_our_direction_case1(nodes[my_node], "right", 1, 0, MOVEDFROM);
            if (to_ind != -1)
                update_our_direction_case1(nodes[to_ind], "left", 3, 4, MOVEDTO);
        }
        else if (move == 'U')
        {
            update_our_direction_case1(nodes[my_node], "top", 1, 0, MOVEDFROM);
            if (to_ind != -1)
                update_our_direction_case1(nodes[to_ind], "bottom", 3, 4, MOVEDTO);
        }
        else if (move == 'D')
        {
            update_our_direction_case1(nodes[my_node], "bottom", 1, 0, MOVEDFROM);
            if (to_ind != -1)
                update_our_direction_case1(nodes[to_ind], "top", 3, 4, MOVEDTO);
        }
    }
    else
    {
        int from_ind = -1;
        int to_ind = -1;
        //СЛУЧАЙ II
        //printf("THERE row %d col %d\n", p_row, p_col);
        for (int i = 0; i < nodes.size(); i++)
        {
            if (is_neighbour(p_row, p_col, nodes[i]))
            {
                int side = get_side(p_row, p_col, nodes[i]);
                //left = 0, right = 1, top = 2, bottom = 3

                if (move == 'L')
                {
                    if (side == 0)
                        to_ind = i;
                    if (side == 1)
                        from_ind = i;
                }

                if (move == 'R')
                {
                    if (side == 1)
                        to_ind = i;
                    if (side == 0)
                        from_ind = i;
                }

                if (move == 'U')
                {
                    if (side == 2)
                        to_ind = i;
                    if (side == 3)
                        from_ind = i;
                }

                if (move == 'D')
                {
                    if (side == 3)
                        to_ind = i;
                    if (side == 2)
                        from_ind = i;
                }

            }
        }


        int from_seg_pos = -1;
        int to_seg_pos = -1;

        if (from_ind != -1)
            from_seg_pos = get_seg_coords(p_row, p_col, nodes[from_ind]);
        if (to_ind != -1)
            to_seg_pos = get_seg_coords(p_row, p_col, nodes[to_ind]);

        if (move == 'L')
        {
            if (from_ind != -1)
                update_direction_case2(nodes[from_ind], "left", from_seg_pos, 1, MOVEDFROM);
            if (to_ind != -1)
                update_direction_case2(nodes[to_ind], "right", to_seg_pos, -1, MOVEDTO);
        }
        else if (move == 'R')
        {
            if (from_ind != -1)
                update_direction_case2(nodes[from_ind], "right", from_seg_pos, 1, MOVEDFROM);
            if (to_ind != -1)
                update_direction_case2(nodes[to_ind], "left", to_seg_pos, -1, MOVEDTO);
        }
        else if (move == 'U')
        {
            if (from_ind != -1)
                update_direction_case2(nodes[from_ind], "top", from_seg_pos, 1, MOVEDFROM);
            if (to_ind != -1)
                update_direction_case2(nodes[to_ind], "bottom", to_seg_pos, -1, MOVEDTO);
        }
        else if (move == 'D')
        {
            if (from_ind != -1)
                update_direction_case2(nodes[from_ind], "bottom", from_seg_pos, 1, MOVEDFROM);
            if (to_ind != -1)
                update_direction_case2(nodes[to_ind], "top", to_seg_pos, -1, MOVEDTO);
        }
        //СЛУЧАЙ III
        //(дополнение к случаю II - просто расставить MOVEDTO на всех инцидентных отрезках в точке [0] кроме nodes[from_ind])

        if (to_ind != -1)
        {
            Coords to_node_coords = nodes[to_ind].get_coords();
            int to_row = to_node_coords.row;
            int to_col = to_node_coords.col;

            Policeman value;//значение в клетке целевого нода, куда мы пришли в случае II
            if (move == 'L')
            {
                value = (nodes[to_ind].get_right_seg_guy())[0];
            }
            else if (move == 'R')
            {
                value = (nodes[to_ind].get_left_seg_guy())[0];
            }
            else if (move == 'U')
            {
                value = (nodes[to_ind].get_bottom_seg_guy())[0];
            }
            else if (move == 'D')
            {
                value = (nodes[to_ind].get_top_seg_guy())[0];
            }

            if (is_getting_to_node(p_row, p_col, move))
            {
                for (int i = 0; i < nodes.size(); i++)
                    if (is_neighbour(to_row, to_col, nodes[i]) && i != from_ind)
                    {
                        int side = get_side(to_row, to_col, nodes[i]);

                        if (side == 0) //left
                        {
                            update_direction_case3(nodes[i], "right", value);
                            update_direction_case3(nodes[to_ind], "left", value);
                        }
                        else if (side == 1) //right
                        {
                            update_direction_case3(nodes[i], "left", value);
                            update_direction_case3(nodes[to_ind], "right", value);
                        }

                        else if (side == 2) //top
                        {
                            update_direction_case3(nodes[i], "bottom", value);
                            update_direction_case3(nodes[to_ind], "top", value);
                        }
                        else if (side == 3) //bottom
                        {
                            update_direction_case3(nodes[i], "top", value);
                            update_direction_case3(nodes[to_ind], "bottom", value);
                        }
                    }
            }
        }
    }

}

vector <Policeman> policemen_from_last_stage(4, Policeman(-1, -1)); //положение полисменов на предыдущем ходе

class HeatMap
{
public:
    vector <vector <int>> heat_map_; //vindetta 

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

        for (int c = 0; c < WIDTH; c += 12)
            for (int r = 0; r < HEIGHT; r++)
                if (r % 4 != 0) heat_map_[r][c] = FROZEN;
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


    // Чёрный квадрат (сверху)
    // Массив 17х25 (посередине)
    // Чёрный квадрат (снизу)

};


HeatMap heat_map;

int player;
int turnes_left;



void init_police_nodes()
{
    // Инициализация ментов
    Coords tfs = translate("AI");
    Coords tsc = translate("AQ");

    /* ФwФ */
    for (int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i].get_coords() == tfs)
        {
            vector<Policeman> p(5);
            vector<int> v = { EMPTY, EMPTY, MOVEDFROM, MOVEDFROM, EMPTY };

            Coords c = translate("AK");
            Coords d = translate("AL");
            p[2] = Policeman(c.row, c.col);
            p[3] = Policeman(d.row, d.col);


            nodes[i].set_right_seg_guy(p);
            nodes[i].set_right_seg_direction(v);
        }
    }

    /* ФwФ */
    for (int i = 0; i < nodes.size(); i++)
    {
        if (nodes[i].get_coords() == tsc)
        {
            vector<Policeman> p(5);
            vector<int> v = { EMPTY, MOVEDFROM, MOVEDFROM, EMPTY, EMPTY };

            Coords c = translate("AN");
            Coords d = translate("AO");
            p[1] = Policeman(c.row, c.col);
            p[2] = Policeman(d.row, d.col);

            nodes[i].set_left_seg_guy(p);
            nodes[i].set_left_seg_direction(v);
        }
    }

    // конец инициализации ментов

}






int main()
{
    srand(time(NULL));
    cin >> turnes_left >> player;

    self_init();
    fill_adj();

    init_police_nodes();

    while (!game_stop())
    {
        input_processor();


        field_situation.check_collisions();
        field_situation.update_onlifts_persons();

        // Ладно, за работу
        if (player == BURGLARS)
        {
            field_situation.check_collisions(); // Были пойманы бандиты
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
            field_situation.check_collisions(); // Были пойманы бандиты
        }

        //!!!!!!
        //вызвать seg_update для изменения положения каждого полицейского.

        print_turn();




        turnes_left--;
        update_adj();
    }
}

/*
#ifdef DEBUG_

        cout << "--------\n" << endl;

        for (auto x : burglars)
        {
            Coords c = x.get_coords();
            printf("|%d %d| ", c.row, c.col);
        }
        cout << endl;

        for (auto x : policemen)
        {
            Coords c = x.get_coords();
            printf("|%d %d| ", c.row, c.col);
        }
        cout << endl;

        cout << "\n--------\n" << endl;


#endif // DEBUG
*/




Coords translate(string pos) //переводит ход из внешнего формата во внутренний
{
    return (pos != "??" ? Coords(pos[0] - 'A', pos[1] - 'A') : Coords(-1, -1));
    //return (pos != "??" ? {pos[1] - 'A', pos[0] - 'A'} : NOINFORMATION);
}

// ?? ?? ?? ?? FFF 
// AJ AM AM AP FFF
// Agent Hitler FBI
void input_processor() //ЧВК П.А.Д.Л.А. (обработчик ввода (распихивает всё по классам))
{
    vector <Coords> positions;


    for (int i = 0; i < PSIZE; ++i) {
        string pos;
        cin >> pos;

        positions.push_back(translate(pos));
    }
    //прочитать пятую строку и использовать метод set_lift_status() (будет ещё перегрузка для него)

    string lift_status;
    cin >> lift_status;

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
            pair<int, int> dif = { positions[i].row - policemen[i].get_coords().row, positions[i].col - policemen[i].get_coords().col };
            if (dif == make_pair(0, 0)) policemen[i].set_move(STANDBY);
            else if (dif == make_pair(0, 1)) policemen[i].set_move(MOVERIGHT);
            else if (dif == make_pair(0, -1)) policemen[i].set_move(MOVELEFT);
            else if (dif == make_pair(1, 0)) policemen[i].set_move(MOVEUP);
            else if (dif == make_pair(-1, 0)) policemen[i].set_move(MOVEDOWN);

            seg_update(policemen[i]);

            policemen[i].set_coords(positions[i]);
        }
    }
}


void self_init() //инициализация в зависимости от того, за кого мы играем
{
    if (player == BURGLARS)
    {
        set_nodes();
        burglars[0].set_coords(translate("GE"));
        burglars[1].set_coords(translate("GQ"));
        burglars[2].set_coords(translate("MW"));
        burglars[3].set_coords(translate("QH"));

        cout << "GE GQ MW QH" << endl;
    }

    policemen[0].set_coords(translate("AK"));
    policemen[1].set_coords(translate("AL"));
    policemen[2].set_coords(translate("AN"));
    policemen[3].set_coords(translate("AO"));
}


void print_turn()
{

    if (player == POLICE)
        for (auto x : policemen)
            cout << x.get_move();
    else
        for (auto x : burglars)
            cout << x.get_move();

    cout << endl;
};


bool game_stop()
{
    return field_situation.burglars_not_caught == 0 || turnes_left == 0;
}

void make_stupid_move(Policeman& p) //Void Voidovitch
{
    //left = 0, right = 1, top = 2, bottom = 3;
    vector <int> res(4);
    Coords coo = p.get_coords();
    int row = coo.row;
    int col = coo.col;

    if (col - 1 >= 0)
        res[0] = 1;
    if (col + 1 < WIDTH)
        res[1] = 1;
    if (row + 1 < HEIGHT)
        res[2] = 1;
    if (row - 1 >= 0)
        res[3] = 1;

    int old_res = p.last_move;

    vector <int> good_res;
    for (int i = 0; i < 4; i++)
        if (res[i] == 1)
            good_res.push_back(i);

    int final_res = good_res[rand() % good_res.size()];

    if (!is_node(row, col)) {
        if (res[old_res] == 1) final_res = old_res;
    }

    int move;
    if (final_res == 0)
        move = MOVELEFT;
    else if (final_res == 1)
        move = MOVERIGHT;
    else if (final_res == 2)
        move = MOVEUP;
    else if (final_res == 3)
        move = MOVEDOWN;
    else
        move = STANDBY;

    p.last_move = final_res;

    p.set_move(move);
    shift_player(move, p);
}

void make_move_police()
{


    for (auto& p : policemen)
    {

        Coords crd = p.get_coords();
        int row = crd.row;
        int col = crd.col;

        vector<pair<int, int>> steps{ { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } }; // thanks; its very kind of you
        vector<int> moves{ MOVEUP, MOVEDOWN, MOVERIGHT, MOVELEFT, STANDBY };

        int res = 4; // res ∈ [0; 4]
        int cur_temp = heat_map.heat_map_[row][col];
        int temp = FROZEN - 1;

        // cout << cur_temp << ' ';



        for (int i = 0; i < 4; i++)
        {
            int nr = row + steps[i].first, nc = col + steps[i].second;

            if (nr >= 0 && nr < HEIGHT && nc >= 0 && nc < WIDTH)
                temp = heat_map.heat_map_[nr][nc];

            if (temp > cur_temp) {
                res = i;
                cur_temp = temp;
            }
        }

        /*if (temp == 0)
        {
            if (col % 4 == 0 && col % 12 != 0)
                res = 4;
        }*/

        //if (temp == 0) make_stupid_move(p);
        //printf("THIS IS RES MTFC %d\n", res);
        //cout << temp << endl;
        p.set_move(moves[res]);
        shift_player(moves[res], p);
        //make_stupid_move(p);

    }
}

int get_policeman_time(Node node, string type)
{
    vector <int> direction;

    if (type == "left")
    {
        direction = node.get_left_seg_direction();
    }
    else if (type == "right")
    {
        direction = node.get_right_seg_direction();
        /*cout << "----DIRECTION----\n";
        for (auto x : direction)
            cout << x << " ";
        cout << "\n----^^^^^^^^^----\n";*/
    }
    else if (type == "top")
    {
        direction = node.get_top_seg_direction();
    }
    else
    {
        direction = node.get_bottom_seg_direction();
    }

    for (int i = 0; i < 5; i++)
        if (direction[i] == MOVEDTO)
        {
            return i;
        }
    return INF;
}

int get_side(int row, int col, Node neighbour)
{
    Coords crd = neighbour.get_coords();
    int neighbour_row = crd.row;
    int neighbour_col = crd.col;

    if (neighbour_row > row)
        return 2;//up
    else if (neighbour_row < row)
        return 3;//bottom
    else if (neighbour_col > col)
        return 1;//right
    else
        return 0;//left

    //индексация - как в массиве res ниже
}
//just give me more power...
//ГОДНОКОД
//GOOD(NO)CODE
int get_seg_coords(int row, int col, Node node)
{
    Coords crd = node.get_coords();
    int nrow = crd.row;
    int ncol = crd.col;

    return abs(nrow - row) + abs(ncol - col);
}

bool is_way_clear(int row, int col, Node node, string type)
{
    vector <int> direction;

    if (type == "left")
    {
        direction = node.get_left_seg_direction();
    }
    else if (type == "right")
    {
        direction = node.get_right_seg_direction();
    }
    else if (type == "top")
    {
        direction = node.get_top_seg_direction();
    }
    else
    {
        direction = node.get_bottom_seg_direction();
    }

    int my_pos = get_seg_coords(row, col, node);

    for (int i = 0; i <= my_pos; i++)
        if (direction[i] != EMPTY)
            return false;
    return true;
}

int my_time(int row, int col, Node node) //ЧТО ЭТО ТАКОЕ У НАС ЖЕ ЕСТЬ get_seg_coords!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
{
    Coords crd = node.get_coords();
    int nrow = crd.row;
    int ncol = crd.col;

    return abs(nrow - row) + abs(ncol - col);
}

/*
┈┈┏╮┏╮┈┈┈┈┈┈┈┈╭╮
┈╭┛┗┛┗┳━━━━━━╮┃┃
┈┃▅┃▅┈┃╰╰╰╰╰╰┣╯┃
▇┻━╯┈┈┃╰╰╰╰╰╰┣━╯
┣━━━╯┈╰╰╰╰╰╰╰┃┈┈
╰━━┳┳━┓┏━┳┳┓┏╯┈┈
┈┈┈┃┃┈┃┃┈┃┃┃┃┈┈┈

*/

void make_move_burglar() //МОЛИТЕСЬ ТУТ БУДЕТ МНОГО БАГОВ. NO MORE.. upd: ARE YOU SURE ABOUT THAT?
{
    for (int i = 0; i < 4; i++)
    {
        if (burglars[i].is_caught())
        {
            burglars[i].set_move(-1);
            continue;
        }

        Coords burglar = burglars[i].get_coords();
        int row = burglar.row;
        int col = burglar.col;

        vector <Node> neighbours;
        vector <LiftNode> lift_neighbours;

        /* (ΦωΦ) */
        for (auto node : adj[{row, col}])
        {
            neighbours.push_back(node);
        }

        for (auto lift : adj_lifts[{row, col}])
        {
            lift_neighbours.push_back(lift);
        }

        //left = 0, right = 1, top = 2, bottom = 3
        vector <int> res(4, 0); //говорит мне, какой из соседних нодов хороший

        for (auto node : neighbours) //проход по соседним узлам(не лифтам)
        {
            Coords nd = node.get_coords();
            int nd_row = nd.row;
            int nd_col = nd.col;

            int left_time = get_policeman_time(node, "left");
            int right_time = get_policeman_time(node, "right");
            int top_time = get_policeman_time(node, "top");
            int bottom_time = get_policeman_time(node, "bottom");
            int our_time = my_time(row, col, node); // Цените время 

            string our_seg_type;
            if (is_our_seg(row, col, nd_row, nd_col, "left"))
                our_seg_type = "left";
            else if (is_our_seg(row, col, nd_row, nd_col, "right"))
                our_seg_type = "right";
            else if (is_our_seg(row, col, nd_row, nd_col, "top"))
                our_seg_type = "top";
            else
                our_seg_type = "bottom";

            int good = 1;
            if (left_time <= our_time || right_time <= our_time || top_time <= our_time || bottom_time <= our_time || !is_way_clear(row, col, node, our_seg_type))
                good = 0;

            int side = get_side(row, col, node);
            res[side] = good;

            //cout << "!!!" << endl;
            //cout << side << endl;
            //cout << left_time << endl;
            //cout << right_time << endl;
            //cout << top_time << endl;
            //cout << bottom_time << endl;
          //  cout << our_time << endl;

            /*cout << "~~~~~~~0_0~~~~~~~" << endl;
            if (i == 3 && side == 1)
            {
                for (auto v : node.get_right_seg_direction())
                    cout << v << ' ';
                cout << endl;
            }
            cout << "~~~~~~~0_0~~~~~~~" << endl;*/
            // cout << "!!!" << endl;

            // for (auto v : node.get_left_seg_direction())
             //    cout << v << " ";
           //  cout << "\n!!!" << endl;


        }


        /* for (auto r : res)
             cout << r << " ";
         cout << endl;*/

        int old_res = burglars[i].last_move;

        vector <int> good_res;
        for (int i = 0; i < 4; i++)
            if (res[i] == 1)
                good_res.push_back(i);

        int final_res = good_res[rand() % good_res.size()];


        if (res[old_res] == 1) final_res = old_res;


        int move = STANDBY; //окончательный ход
        if (final_res == 0)
            move = MOVELEFT;
        else if (final_res == 1)
            move = MOVERIGHT;
        else if (final_res == 2)
            move = MOVEUP;
        else if (final_res == 3)
            move = MOVEDOWN;


        burglars[i].last_move = final_res;


        if (move == STANDBY)
            if (!is_lift(row, col))
                for (auto lift : lift_neighbours) //проход по соседним лифтам(c учётом того, что мы не можем вызвать лифт)
                {
                    if (lift.is_using())
                        continue;

                    Coords nd = lift.get_coords();
                    int nd_row = nd.row;
                    int nd_col = nd.col;
                    int res = STANDBY;

                    if (nd_col > col)
                        res = MOVERIGHT;
                    else
                        res = MOVELEFT;

                    for (auto floor : lift.get_floors())
                    {
                        if (floor == EMPTY)
                        {
                            move = res;
                            break;
                        }
                    }
                }
            else
            {
                LiftNode our_lift(0);


                for (auto lift : lift_nodes)
                {
                    Coords nd = { row, col };
                    if (lift.get_coords() == nd)
                        our_lift = lift;
                }

                if (!our_lift.is_using())
                {
                    for (auto floor : our_lift.get_floors())
                        //проход по соседним лифтам(с учётом того, что мы вызываем лифт) (нужно будет ещё дописать проверку на то, что лифт не используется)
                    {
                        if (floor == EMPTY && floor != row / 4)
                        {
                            move = our_lift.get_floor() + 1;
                            break;
                        }
                    }
                }
            }
        if (move <= 5 && move >= 1) //если вызвали лифт, то входим в него и помечаем что он занят (нужно будет создать что-то, чтобы помечать занятость лифта глобально)
            burglars[i].enter_in_lift(abs(row / 4 + 1 - move) * 2);

        burglars[i].set_move(move); //дальше бога нет...
        shift_player(move, burglars[i]);
    }
}

bool is_lift(int row, int col)
{
    return row % 4 == 0 && col % 12 == 0;
}

void shift_player(int step, Burglar& burg)
{
    Coords crd = burg.get_coords();

    if (step == MOVERIGHT)
        crd.col++;
    else if (step == MOVELEFT)
        crd.col--;
    else if (step == MOVEUP)
        crd.row++;
    else if (step == MOVEDOWN)
        crd.row--;

    burg.set_coords(crd);
}

void shift_player(int step, Policeman& pol)
{
    Coords crd = pol.get_coords();

    if (step == MOVERIGHT)
        crd.col++;
    else if (step == MOVELEFT)
        crd.col--;
    else if (step == MOVEUP)
        crd.row++;
    else if (step == MOVEDOWN)
        crd.row--;

    pol.set_coords(crd);
}

bool is_our_seg(int row, int col, int nd_row, int nd_col, string type)
{
    if (col < nd_col && type == "left")
        return true;
    if (col > nd_col && type == "right")
        return true;
    if (row < nd_row && type == "bottom")
        return true;
    if (row > nd_row && type == "top")
        return true;

    return false;
}

void update_adj()
{
    for (auto it = adj.begin(); it != adj.end(); ++it) //перепресвоить adj[key] напрямую
    {
        auto key = it->first;

        //вариант покороче
        for (int i = 0; i < adj[key].size(); ++i)
            for (int j = 0; j < nodes.size(); ++j)
                if (nodes[j].get_coords() == adj[key][i].get_coords())
                    adj[key][i] = nodes[j];
    }
}
/*


AJ AM AM AP FFF
 */







