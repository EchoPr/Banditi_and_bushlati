#include <iostream>
#include <string>
#include <vector>

#define l_status int

using namespace std;

const int LFREE = 0;
const int LBUSY = 1;

struct Coords
{
	int row, col;

	Coords(int r, int c) { row = r; col = c; }
};


class FieldSituation
{
public:
	l_status l_lift, m_lift, r_lift;
	int buglars_not_caught;

	FieldSituation()
	{
		buglars_not_caught = 4;
		l_lift = m_lift = r_lift = LFREE;
	}
	
};


class Person
{
protected:
	static Coords coords_;
	static bool in_lift;
	static int steps_in_lift_left;
};


class Buglar : public Person
{
public:
	Buglar(int x, int y)
	{
		coords_ = Coords(x, y);
		in_lift = false;
		caught = false;
		steps_in_lift_left = 0;
	}

private:
	bool caught;
};


class Policeman : public Person
{
public:
	Policeman(int x, int y)
	{
		coords_ = Coords(x, y);
		in_lift = false;
		steps_in_lift_left = 0;
	}
};


vector <Buglar> buglars;
vector <Policeman> policemen;

int player;
int turnes_left;

void set_buglar_positions();

string get_status();
void print_turn();

bool game_stop();

void make_move(string turn);

int main()
{
	cin >> turnes_left >> player;

	FieldSituation field_situaition();

	if (player)
		set_buglar_positions();

	while (!game_stop())
	{
		make_move(get_status());
		print_turn();

		turnes_left--;
	}
} 


void set_buglar_positions()
{
	cout << "AA AB AC AD" << endl;
}


string get_status() 
{
	string turn;
	cin >> turn;

	return turn;
};


void print_turn(string turn) 
{
	cout << turn << endl;
};


bool game_stop(FieldSituation fs)
{
	return fs.buglars_not_caught == 0 || turnes_left == 0;
}

void make_move(string turn) 
{

}


