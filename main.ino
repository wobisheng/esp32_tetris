#include "LedControl.h"
#include <arduino-timer.h>

auto timer = timer_create_default();
LedControl lc=LedControl(36,40,26,1);

int shape = 0;
bool Map[8][8] = { 0 };
int shape_map[7][3][2] = {
                      {{0,-1},{1,-1},{1,0}},//方块
                      {{0,-1},{-1,0},{1,0}},//T形
                      {{0,-2},{0,-1},{1,0}},//L形
                      {{0,-2},{0,-1},{0,1}},//|形
                      {{-1,0},{0,-1},{1,-1}},//Z形
                      {{0,-2},{0,-1},{-1,0}},//对称L
                      {{-1,-1},{0,-1},{1,0}}//对称Z
                      }; 
int store_x[4] = { 0 };
int store_y[4] = { 0 };

void Draw();
bool judge_bound();
void Rotate();
bool AutoMove(void *);
bool judge_finish();
void right();
void left();
void Convert(int * new_x,int *new_y);

void setup() 
{
  Serial.begin(115200);
  lc.shutdown(0,false);
  lc.setIntensity(0,5);
  lc.clearDisplay(0);
  summon();
  timer.every(2000, AutoMove);
}

void loop() 
{
  if(Serial.available()){
        String MyString = Serial.readString();
        char myString[MyString.length()+1];
        strcpy(myString,MyString.c_str());
        if (myString[0] == 'r') {right();Serial.println("r");}
        if (myString[0] == 'l') {left();Serial.println("l");}
        if (myString[0] == 'o') {Rotate();Serial.println("o");}
    }
    timer.tick();
}

bool judge_bound(int * x,int *y)
{
  for (int i =0 ; i <4 ; i ++)
  {
    if(x[i] < 0||x[i] > 7)
    { return false; }
    Map[store_y[i]][store_x[i]] = 0; 
  }
  for (int i = 0; i < 4; i ++)
  {
    if(Map[y[i]][x[i]] == 1)
    {return false;} 
  }
  return true;
}

bool judge_bound(int * coordinate)
{
  for (int i = 0; i < 4 ; i ++)
  {
    if(coordinate[i] < 0||coordinate[i] > 7||Map[coordinate[i]][store_y[i]] == 1)
    { return false; }
  }
  return true;
}

bool judge_finish()
{
  int temp = 0;
  for (int i = 0; i <= 3 ; i ++)
  {
    if (store_y[i] == 7)
    {return true;}
    if (store_y[i]>= 0)
    {
      if (store_y[i] >= temp)
      { temp = store_y[i]; }
    }
  }
  for (int i = 0; i < 4; i ++ )
  {
    if (store_y[i] == temp)
    {
      if (Map[store_y[i] + 1][store_x[i]]) return true;
    }
  }
  return false;
}

void summon()
{
  shape = random(1,8);
  Serial.println(shape);
  store_x[0] =  3;
  store_y[0] =  0;
  store_x[1] =  3 + shape_map[shape][0][0];
  store_y[1] =  0 + shape_map[shape][0][1];
  store_x[2] =  3 + shape_map[shape][1][0];
  store_y[2] =  0 + shape_map[shape][1][1];
  store_x[3] =  3 + shape_map[shape][2][0];
  store_y[3] =  0 + shape_map[shape][2][1];
}

void Rotate()
{
  int temp_x[4],temp_y[4];
  temp_x[0] = store_x[0];
  temp_y[0] = store_y[0];
  for (int i = 1; i < 4; i ++)
  {
    temp_x[i] = store_x[i];
    temp_y[i] = store_y[i];
  }
  for (int i = 1; i < 4; i ++)
  {
      int minus_x = temp_x[i] - temp_x[0];
      int minus_y = temp_y[i] - temp_y[0];
      if (minus_x == 0)
      { minus_x = - minus_y; minus_y = 0;}
      else if (minus_y == 0)
      { minus_y = minus_x; minus_x = 0;}
       else if (minus_x > 0&& minus_y > 0)
       { minus_x = - minus_x; }
       else if (minus_x < 0 && minus_y > 0)
       {minus_y = - minus_y;}
       else if (minus_x< 0 && minus_y < 0)
       {minus_x = - minus_x;}
       else if (minus_x > 0 && minus_y < 0)
       {minus_y = - minus_y;}
        temp_x[i] = temp_x[0] + minus_x;
        temp_y[i] = temp_y[0] + minus_y;
   }
   if (judge_bound(temp_x,temp_y))
   {
    for (int i =0 ; i <4 ; i ++)
    {
      Map[store_y[i]][store_x[i]] = 1;
    }
    Convert(temp_x,temp_y);
    for (int i = 1; i < 4; i ++)
    {store_x[i] = temp_x[i]; store_y[i] = temp_y[i]; }
   }
}

bool AutoMove(void *)
{
  int temp_y[4];
  for (int i = 0; i < 4; i ++)
  {
    temp_y[i] = store_y[i] + 1;
  }
  Convert(store_x,temp_y);
  for (int i = 0; i < 4; i ++)
  {
    store_y[i] = temp_y[i];
  }
  if(judge_finish())
  {
    summon();
  }
  return true;
}

void right()
{
  int temp[4];
  for (int i = 0; i < 4; i ++)
  {
    temp[i] = store_x[i] + 1;
  }
  if(judge_bound(temp))
  {
    Convert(temp,store_y);
    for (int i = 0; i < 4; i ++)
    {
      store_x[i] ++;
    }
  }
}

void left()
{
  int temp[4];
  for (int i = 0; i < 4; i ++)
  {
    temp[i] = store_x[i] - 1;
  }
  if(judge_bound(temp))
  {
    Convert(temp,store_y);
    for (int i = 0; i < 4; i ++)
    {
      store_x[i] --;
    }
  }
}

void Convert(int * new_x,int *new_y)
{
      for (int i = 0; i < 4; i ++)
      {
        if (store_y[i] >= 0)
        Map[store_y[i]][store_x[i]] = 0;
      }
      for (int i =0; i < 4; i ++)
      {
        if (new_y[i]>= 0)
        Map[new_y[i]][new_x[i]] = 1;
      }
    Draw();
}

void Draw()
{
  for (int j = 0; j <=7; j ++)
  {
    int temp = 0;
    for (int i = 0; i <= 7; i ++)
    {
      if (Map[j][i]) {temp += pow(2,7-i);}
      if (i == 7) {lc. setColumn(0,7-j,temp);}
    }
  }
}
