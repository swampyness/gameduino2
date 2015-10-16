#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>
#include "tile.h"
#define MAP_WIDTH			31
#define MAP_HEIGHT			31

float delta = 180.0 / 3.14159265358979323846;

byte data_field[MAP_WIDTH][MAP_HEIGHT];
short dir_x[4];
short dir_y[4];

struct _shape {
	int x[4];
	int y[4];
};
_shape shape[1];

struct _pt {
	int id;
	int x;
	int y;
	int x1;
	int y1;
	int width;
	int height;
	int width2;
	int height2;
	int angle;
	int radius;
	int speed;
	int speed_max;
	int timer;
};
int pts = 1;
_pt pt[20];
int ens = 0;
_pt en[200];
_pt screen;
_pt field;
_pt touch;

Poly po;

void reset_maze() {
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			data_field[x][y] = 0;
		}
	}
}

boolean is_free(int xpos, int ypos) {
	if ((xpos < MAP_WIDTH - 1) && (xpos >= 1) && (ypos < MAP_HEIGHT - 1) && (ypos >= 1)) {
		return !data_field[xpos][ypos];
	} else {
		return false;
	}
}

void random_directions() {
	int randdirection;
	randdirection = (int)(random(0, 4));
	for (byte i = 0; i < 4; i++) {
		dir_x[i] = 0;
		dir_y[i] = 0;
	}
	if (randdirection == 0) {
		dir_x[0] = -1;
		dir_x[1] = 1;
		dir_y[2] = -1;
		dir_y[3] = 1;
	}
	if (randdirection == 1) {
		dir_x[3] = -1;
		dir_x[2] = 1;
		dir_y[1] = -1;
		dir_y[0] = 1;
	}
	if (randdirection == 2) {
		dir_x[2] = -1;
		dir_x[3] = 1;
		dir_y[0] = -1;
		dir_y[1] = 1;
	}
	if (randdirection == 3) {
		dir_x[1] = -1;
		dir_x[0] = 1;
		dir_y[3] = -1;
		dir_y[2] = 1;
	}
}

void generate_maze() {
	randomSeed(analogRead(0));
	int cell_nx = 0;
	int cell_ny = 0;
	int cell_sx = 0;
	int cell_sy = 0;
	int dir = 0;
	int done = 0;
	boolean filled = false;
	boolean blocked = false;
	do {
		do {
			cell_sx = (int)random(1, (MAP_WIDTH));
		} while (cell_sx % 2 == 0);
		do {
			cell_sy = (int)random(1, (MAP_HEIGHT));
		} while (cell_sy % 2 == 0);
		if (done == 0) data_field[cell_sx][cell_sy] = true;
		if (data_field[cell_sx][cell_sy]) {
			do {
				random_directions();
				blocked = true;
				filled = false;
				for (dir = 0; (dir < 4); dir++) {
					if (!filled) {
						cell_nx = cell_sx + (dir_x[dir] * 2);
						cell_ny = cell_sy + (dir_y[dir] * 2);
						if (is_free(cell_nx, cell_ny)) {
							data_field[cell_nx][cell_ny] = true;
							data_field[cell_sx + dir_x[dir]][cell_sy + dir_y[dir]] = true;
							cell_sx = cell_nx;
							cell_sy = cell_ny;
							blocked = false;
							done++;
							filled = true;
						}
					}
				}
			} while (!blocked);
		}
	} while (((done + 1) < (((MAP_WIDTH - 1) * (MAP_HEIGHT - 1)) / 4)));
}

void invert_maze() {
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for (int x = 0; x < MAP_WIDTH; x++) {
			if (data_field[x][y] != 0) {
				data_field[x][y] = 0;
			} else {
				data_field[x][y] = 1;
			}
		}
	}
}

void form_gap() {
	int plot_x, plot_y, block_x, block_y, block_width, block_height;
	block_width = MAP_WIDTH * 0.1;
	block_height = MAP_HEIGHT * 0.2;
	int suburb = 48;
	if (suburb < 0) suburb = 0;
	if (suburb > 58) suburb = 58;
	for (byte i = 0; i < 64 - suburb; i++) {
		block_x = random(MAP_WIDTH - block_width);
		block_y = random(MAP_HEIGHT - block_height);
		for (int plot_y = block_y; plot_y < block_y + block_height; plot_y++) {
			for (int plot_x = block_x; plot_x < block_x + block_width; plot_x++) {
				if (plot_x > 0 && plot_x < MAP_WIDTH - 1 && plot_y > 0 && plot_y < MAP_HEIGHT - 1) {
					data_field[plot_x][plot_y] = 0;
				}
			}
		}
	}
}

void make_maze() {
	reset_maze();
	generate_maze();
	invert_maze();
	form_gap();
}

void setup_init() {
	screen.width = 480;
	screen.height = 240;
	screen.width2 = screen.width / 2;
	screen.height2 = screen.height / 2;

	touch.x1 = screen.width2;
	touch.y1 = screen.height2;
	touch.width = touch.x1 / 3;
	touch.height = touch.y1 / 3;

	field.x1 = 32;
	field.y1 = 32;

	shape[0].x[0] = 1;
	shape[0].y[0] = 1;

	shape[0].x[1] = 0;
	shape[0].y[1] = -1;

	shape[0].x[2] = -1;
	shape[0].y[2] = 1;

	_pt p;
	p.radius = 8;
	po.begin();
	for (int i = 0; i < 3; i++) {
		p.x = shape[0].x[i] * 16 * p.radius;
		p.y = shape[0].y[i] * 16 * p.radius;
		p.x += screen.width2 * 16;
		p.y += screen.height2 * 16;
		po.v(p.x, p.y);
	}

	pt[0].speed_max = 2.0;
	pt[0].x = 16 * 16;
	pt[0].y = 16 * 16;
}


void setup() {
	GD.begin();
	LOAD_ASSETS();
	setup_init();
	make_maze();
	setup_enemy();
}

//void setpixel(byte x, byte y, byte color) {
//  /*
//   Because of the way the sprites are laid out in setup(), it's not too
//   hard to translate the pixel (x,y) to an address and mask.  Taking the
//   two byte values as x7-x0 and y7-y0, the address of the pixel is:
//
//      x5 x4 y7 y6 y5 y4 y3 y2 y1 y0 x3 x2 x1 x0
//
//  (x6, x7) gives the value of the mask.
//  */
//  unsigned int addr = RAM_SPRIMG | (x & 0xf) | (y << 4) | ((x & 0x30) << 8);
//  byte mask = 0xc0 >> ((x >> 5) & 6);
//  GD.wr(addr, (GD.rd(addr) & ~mask) | (color & mask));
//}

boolean on_screen(struct _pt a) {
	return (a.x >= screen.x - field.x1 && 
			a.x <= screen.x + screen.width + field.x1 &&
			a.y >= screen.y - field.y1 &&
			a.y <= screen.y + screen.height + field.y1);
}

void draw_field() {
	_pt p;
	GD.ColorRGB(0xffffff);
	GD.Begin(BITMAPS);
	for (p.y1 = 0; p.y1 < MAP_HEIGHT; p.y1++) {
		for (p.x1 = 0; p.x1 < MAP_WIDTH; p.x1++) {
			if (data_field[p.x1][p.y1] != 0) {
				p.x = (p.x1 * field.x1) - pt[0].x;
				p.y = (p.y1 * field.y1) - pt[0].y;
				p.x += screen.width2;
				p.y += screen.height2;
				if (on_screen(p)) GD.Vertex2f(16 * p.x, 16 * p.y);
			}
		}
	}
}

void draw_play() {
	GD.ColorRGB(0x333344);
	po.draw();
}

void draw_ammo(int id) {
	_pt p;
	p.x = pt[id].x - pt[0].x;
	p.y = pt[id].y - pt[0].y;
	p.x += screen.width2;
	p.y += screen.height2;
	if (on_screen(p)) {
		GD.PointSize(16 * 2);
		GD.Begin(POINTS);
		GD.Vertex2ii(p.x, p.y);
	}
}

void ammo_add() {
	if (pts < 20) {
		pt[pts].x = pt[0].x;
		pt[pts].y = pt[0].y;
		pt[pts].speed_max = pt[0].speed_max * 4;
		pt[pts].angle = pt[0].angle;
		pt[pts].timer = 10;
		pts++;
	} else {
		pts = 1;
	}
}

int calc_play() {
	int t_flag = 0;
	GD.get_inputs();
	touch.x = (GD.inputs.x << 4) / 16;
	touch.y = (GD.inputs.y << 4) / 16;
	if (touch.x > 0 && touch.y > 0) {
		if (touch.y < touch.y1 - touch.height) {
			t_flag = 1;
			if (touch.x < touch.x1 - touch.width) {
				touch.angle = 225;
			}
			else if (touch.x > touch.x1 + touch.width) {
				touch.angle = 315;
			}
			else {
				touch.angle = 270;
			}
		}
		else if (touch.y > touch.y1 + touch.height) {
			t_flag = 1;
			if (touch.x < touch.x1 - touch.width) {
				touch.angle = 135;
			}
			else if (touch.x > touch.x1 + touch.width) {
				touch.angle = 45;
			}
			else {
				touch.angle = 90;
			}
		} else {
			t_flag = 1;
			if (touch.x < touch.x1 - touch.width) {
				touch.angle = 180;
			} else if (touch.x > touch.x1 + touch.width) {
				touch.angle = 0;
			} else {
				t_flag = 0;
			}
		}
	}
	return t_flag;
}

void loop_play() {
	GD.ColorRGB(0xffff00);
	int a_flag = 0;
	int p_flag = calc_play();
	pt[0].angle = touch.angle;
	_pt t;
	for (int p = 0; p < pts; p++) {
		if ((p == 0 && p_flag == 1) || p > 0) {
			t.x = pt[p].x;
			t.x += cos(pt[p].angle / delta) * pt[p].speed_max;
			t.y = pt[p].y;
			t.y += sin(pt[p].angle / delta) * pt[p].speed_max;
			t.x1 = t.x / field.x1;
			t.y1 = t.y / field.y1;
			t.id = data_field[t.x1][t.y1];
			if (t.id == 0) {
				if (p == 0 ) {
					pt[p].angle = touch.angle;
					pt[p].x = t.x;
					pt[p].y = t.y;
					if (pt[p].timer == 0 && p_flag == 1) {
						pt[p].timer = 10;
						a_flag = 1;
					}
				} else {
					pt[p].x = t.x;
					pt[p].y = t.y;
				}
			} else {
				if (p == 0) {

				} else {
					pt[p].speed_max = 0;
				}
			}
		}
		if (pt[p].timer > 0) pt[p].timer--;
		if (p > 0) if (pt[p].speed_max != 0) draw_ammo(p);
	}
	if (a_flag == 1) ammo_add();
}

void enemy_add(struct _pt p) {
	en[ens].x = p.x * field.x1;
	en[ens].y = p.y * field.y1;
	en[ens].x += field.x1 / 2;
	en[ens].y += field.y1 / 2;
	en[ens].speed_max = 2.0;
	ens++;
}

void setup_enemy() {
	_pt p;
	for (int t = 0; t < 40; t++) {
		do {
			p.x = random(MAP_WIDTH / 2);
			p.y = random(MAP_HEIGHT / 2);
			p.x += MAP_WIDTH / 4;
			p.y += MAP_HEIGHT / 4;
		} while (data_field[p.x][p.y] == 1);
		enemy_add(p);
	}
}

void draw_enemy(int id) {
	_pt p;

	p.x = en[id].x - pt[0].x;
	p.y = en[id].y - pt[0].y;
	p.x += screen.width2;
	p.y += screen.height2;
	if (on_screen(p)) {
		GD.PointSize(16 * 10);
		GD.Begin(POINTS);
		GD.Vertex2ii(p.x, p.y);
	}
}

int get_distance(struct _pt a, struct _pt b) {
	_pt c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return sqrt((c.x * c.x) + (c.y * c.y));
}

int enemy_hit(int id) {
	_pt p;
	p.x1 = 0;
	for (int a = 1; a < pts; a++) {
		if (pt[a].speed_max != 0) {
			p.x = get_distance(pt[a], en[id]);
			if (p.x > 0 && p.x < 16) {
				pt[a].speed_max = 0;
				p.x1 = 1;
				return p.x1;
			}
		}
	}
	return p.x1;
}

void loop_enemy() {
	GD.ColorRGB(0x005599);
	_pt t;
	for (int e = 0; e < ens; e++) {
		if (en[e].speed_max != 0) {
			t.x = en[e].x;
			t.x += cos(en[e].angle / delta) * en[e].speed_max * 32;
			t.y = en[e].y;
			t.y += sin(en[e].angle / delta) * en[e].speed_max * 32;
			t.x1 = t.x / field.x1;
			t.y1 = t.y / field.y1;
			t.id = data_field[t.x1][t.y1];
			if (t.id == 0 || random(100) == 1) {
				t.x = en[e].x;
				t.x += cos(en[e].angle / delta) * en[e].speed_max;
				t.y = en[e].y;
				t.y += sin(en[e].angle / delta) * en[e].speed_max;
				en[e].x = t.x;
				en[e].y = t.y;
			} else {
				en[e].angle = random(9) * 45;
			}
			draw_enemy(e);
			if (enemy_hit(e) == 1) {
				en[e].speed_max = 0;
			}
		}
	}
}

void loop() {
	GD.ClearColorRGB(0x778877);
	GD.Clear();
	draw_field();
	loop_enemy();

	loop_play();

	draw_play();
	GD.swap();
}
