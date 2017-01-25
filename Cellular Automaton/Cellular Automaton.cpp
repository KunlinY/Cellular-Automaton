// Cellular Automaton.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <graphics.h>
#include <conio.h>

#define ROUTE				3
#define WIDTH				50
#define LENGTH				1500
#define UNIT				10
#define VIEW				100
#define TURN_VELOCITY		5
#define INIT_VELOCITY		10
#define MAX_VELOCITY		27
#define MAX_ACCELERATION	2.5
#define COLOR_SELFDRIVEING	GREEN
#define COLOR_NORMAL		BLUE
#define COLOR_TURN			RED
#define COLOR_ROAD			LIGHTGRAY
#define TIME				3600


using namespace std;

ofstream out("data.txt");
int seconds = 0;
int trafficCount = 3000;
int turnRate = 2;
int selfRate = 5;
int outNum, turnCar, totalNum;

unsigned long seed;
void randSeed(unsigned long item) {
	seed = item;
}

unsigned long random() {
	return (seed * rand()) % ((1 << 31) - 1);
}

struct Car {
	Car() : isVacant(true) {}
	void create(int index) {
		velocity = INIT_VELOCITY;
		acceleration = 0.0;
		goTurning = false;
		position = 0.0;
		route = index;

		if ((random() % 10) + 1 <= turnRate)
			turn = true;
		else
			turn = false;

		if ((random() % 10) + 1 <= selfRate)
			selfDriving = true;
		else
			selfDriving = false;
		num++;
		isVacant = false;
		rank = num;
	}
	void destroy() {
		isVacant = true;
		num--;
		if (seconds > 100) {
			outNum++;
		}
	}

	double velocity;
	double acceleration;
	double position;
	int route;

	int rank;
	bool isVacant;
	bool turn;
	bool goTurning;
	bool selfDriving;
	static int num;
};
int Car::num = 0;
Car* road[ROUTE + 1][LENGTH / UNIT + 1];
vector<Car> cars(ROUTE * LENGTH / UNIT + 10);

void print(int route, int pos) {
	Car &car = *road[route][pos];
	out << car.rank << ends
		<< "route:" << car.route << ends << "position" << car.position << endl
		<< "self-driving:" << car.selfDriving << ends
		<< "turn:" << car.turn << ends
		<< "goTurning:" << car.goTurning << ends
		<< "velocity:" << car.velocity << ends
		<< "acceleration" << car.acceleration << endl;
}

void initMap() {
	initgraph(LENGTH * 1.5 + 100, WIDTH * ROUTE + 100);
	setbkcolor(WHITE);
	setfillcolor(COLOR_ROAD);
	solidrectangle(50, 50, LENGTH * 1.5 + 50, WIDTH * ROUTE + 50);
}

double combination(int n, int k) {
	if (n < k) {
		return 1;
	}

	int a = 1, b = 1;
	for (int i = n; i > n - k; i--) {
		a *= i;
	}
	for (int i = k; i > 0; i--) {
		b *= i;
	}

	if (n == 0) {
		a = 1;
	}
	if (k == 0) {
		b = 1;
	}

	return (double)(a / b);
}

double b(int k, int n, double p) {
	return combination(n, k) * pow(p, k) * pow(1 - p, n - k);
}

int getCarNum() {
	static int p[ROUTE + 1];
	p[0] = 0;
	p[1] = 1000 * b(1, ROUTE, (double)trafficCount / (double)(TIME * ROUTE));
	p[2] = p[1] + 1000 * b(2, ROUTE, (double)trafficCount / (double)(TIME * ROUTE));
	p[3] = p[2] + 1000 * b(3, ROUTE, (double)trafficCount / (double)(TIME * ROUTE));

	//srand(time(NULL));
	int seed = rand() % 1000;
	for (int i = ROUTE; i >= 0; i--) {
		if (seed >= p[i])
			return i;
	}
	return 0;
}

void generateCar() {
	int num = getCarNum();
	bool flag[ROUTE];
	if (seconds > 100) 
		totalNum += num;
	
	memset(flag, 0, ROUTE * sizeof(bool));

	for (int i = 0; i < num; i++) {
		int index = rand() % ROUTE;
		while (flag[index]) {
			index = rand() % ROUTE;
		}
		flag[index] = true;
		
		for (auto &r : cars) {
			if (r.isVacant) {
				r.create(index);
				break;
			}
		}
	}
}

int goTurningPos(int route, int pos) {
	if (route == 0)
		return NULL;

	route--;
	int i = pos + 1;

	if (i > LENGTH / UNIT)
		return NULL;

	if (road[route][i])
		if (road[route][i]->goTurning)
			return 1;
	if (road[route][i + 1])
		if (road[route][i + 1]->goTurning)
			return 2;
	return NULL;
}

int findFrontCar(int route, int pos) {
	if (road[route][pos + 1])
		return 1;
	if (road[route][pos + 2])
		return 2;
	return NULL;
}

void goStraight(int route, int pos) {
	Car &car = *road[route][pos];
	int front = findFrontCar(route, pos);
	if (front) 
		car.acceleration = (road[route][pos]->position - car.position) * 0.1;
	else 
		car.acceleration = MAX_ACCELERATION;

	car.velocity += car.acceleration;
	if (car.velocity > MAX_VELOCITY) 
		car.velocity = MAX_VELOCITY;
	if (front == 1)
		if (car.velocity > road[route][pos]->velocity) {
			car.velocity = road[route][pos]->velocity;
			car.acceleration = road[route][pos]->acceleration;
		}
	car.position += car.velocity;
	if (car.position > LENGTH)
		road[route][pos]->destroy();

	out << "goStraight" << endl
		<< "velocity" << road[route][pos]->velocity << ends
		<< "position" << road[route][pos]->position << ends
		<< "acceleration" << road[route][pos]->acceleration << endl;
}

void toSelf(int route, int pos) {
	road[route][pos]->velocity *= 5.0 / 6.0;
	road[route][pos]->position += road[route][pos]->velocity;

	out << "toSelf" << endl
		<< "velocity" << road[route][pos]->velocity << ends
		<< "position" << road[route][pos]->position << endl;
}

void toNo(int route, int pos) {
	road[route][pos]->velocity *= 2.0 / 3.0;
	road[route][pos]->position += road[route][pos]->velocity;

	out << "toNo" << endl
		<< "velocity" << road[route][pos]->velocity << ends
		<< "position" << road[route][pos]->position << endl;
}

bool startTurning(int route, int pos) {
	double end = LENGTH - VIEW;
	double position = road[route][pos]->position;
	double p = 1000 * position / end;
	if (position > end)
		p = 1000.0;
	int temp = rand() % 1000;
	out << "startTurning" << endl
		<< "p:" << p << ends
		<< "seed:" << temp << endl;
	return temp < p;
}

bool enoughSpace(int route, int pos) {
	route++;
	out << "enoughSpace" << endl
		<< !findFrontCar(route, pos) << !road[route][pos] << endl;
	return !findFrontCar(route, pos) && !road[route][pos];
}

void backResponse(int route, int pos) {
	road[route][pos]->velocity *= 4.0 / 5.0;
	out << "backResponse" << endl
		<< "velocity" << road[route][pos]->velocity << endl;
}

bool closeToCorner(int route, int pos) {
	double end = LENGTH;
	double s = road[route][pos]->velocity * (road[route][pos]->velocity - TURN_VELOCITY) - 0.5 * (road[route][pos]->velocity - TURN_VELOCITY) * (road[route][pos]->velocity - TURN_VELOCITY);
	out << "closeToCorner" << endl
		<< "s:" << s << ends
		<< (s > end - pos) << endl;
	return s > end - pos;
}

void slowdown(int route, int pos) {
	Car &car = *road[route][pos];
	car.acceleration = -1;
	car.velocity += car.acceleration;
	if (car.velocity < TURN_VELOCITY)
		car.velocity = TURN_VELOCITY;
	car.position += car.velocity;
	if (car.position > LENGTH)
		road[route][pos]->destroy();

	out << "slowdown" << endl
		<< "velocity:" << car.velocity << ends
		<< "position:" << car.position << endl;
}

void drive(int route, int pos) {
	print(route, pos);
	if (road[route][pos]->turn == false) {
		int turnCar = goTurningPos(route, pos);
		out << "goTurningPos" << turnCar << endl;
		if (turnCar) {
			if (road[route][pos]->selfDriving)
				toSelf(route, pos);
			else
				toNo(route, pos);
		}
		else 
			goStraight(route, pos);
		return;
	}
	if (route != ROUTE - 1) {
		out << "ROUTE - 1" << endl;
		if (road[route][pos]->goTurning == false) {
			if (road[route][pos]->selfDriving)
				road[route][pos]->goTurning = true;
			else if (startTurning(route, pos)) 
					road[route][pos]->goTurning = true;
		}
		else {
			if (enoughSpace(route, pos)) {
				road[route][pos]->route++;
				turnCar++;
			}
			else
				backResponse(route, pos);
		}
		goStraight(route, pos);
		return;
	}
	else {
		if (closeToCorner(route, pos)) {
			slowdown(route, pos);
		}
		else {
			goStraight(route, pos);
		}
		return;
	}
}

void move() {
	memset(road, NULL, sizeof(road));

	for (int i = 0; i < cars.size(); i++) {
		if (!cars[i].isVacant) {
			road[cars[i].route][(int)(cars[i].position / UNIT)] = &cars[i];
		}
	}
	out.open("data.txt");
	for (int i = LENGTH / UNIT; i >= 0; i--) {
		for (int j = ROUTE - 1; j >= 0; j--) {
			if (road[j][i]) {
				drive(j, i);
				out << endl;
			}
		}
	}
	out << endl << endl;
	out.close();
}

void draw() {
	cleardevice();
	for (int i = 0; i < ROUTE; i++) {
		for (int j = 0; j < LENGTH / UNIT + 1; j++) {
			int color = 0x00;

			if (road[i][j]) {
				if (road[i][j]->selfDriving)
					color |= COLOR_SELFDRIVEING;
				else
					color |= COLOR_NORMAL;
				if (road[i][j]->turn)
					color |= COLOR_TURN;
			}
			else
				color = COLOR_ROAD;

			setfillcolor(color);
			solidrectangle(50 + j * UNIT, 50 + i * WIDTH, 50 + (j + 1) * UNIT, 50 + (i + 1) * WIDTH);
			if (road[i][j]) {
				settextcolor(BLACK);
				char str[5];
				itoa(road[i][j]->rank, str, 10);
				outtextxy(50 + j * UNIT, 50 + i * WIDTH, str);
			}
		}
	}
}

void destroyAll() {
	memset(road, NULL, sizeof(road));
	for (auto &r : cars)
		r.destroy();
	Car::num = 0;
	outNum = 0;
	turnCar = 0;
	totalNum = 0;
	seconds = 0;
}

int main()
{
	seed = time(NULL);
	initMap();

	/*ofstream oo("final.csv");
	oo << "Traffic Count, Self-driving Car Rate, Turning Rate, Time, Total Num, Traffic Flow, Actual Turning Num" << endl;
	for (; trafficCount < 6500; trafficCount += 500) {
		for (selfRate = 0; selfRate <= 10; selfRate++) {
			for (turnRate = 0; turnRate <= 10; turnRate++) {
				cout << trafficCount << "\t" << selfRate << "\t" << turnRate << endl;
				oo << trafficCount << ", " << selfRate << ", " << turnRate << ", " << 3500 << ", ";
				while (seconds++ < TIME) {
					generateCar();
					move();
					draw();
				}
				oo << totalNum << ", " << outNum << ", " << turnCar << endl;
				destroyAll();
			}
		}
	}*/

	while (seconds++ < TIME) {
		generateCar();
		move();
		draw();
	}

	_getch();
    return 0;
}

