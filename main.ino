#include "LedControl.h"
#include <arduino-timer.h>
#include <WiFi.h>
#include <WebServer.h>

auto timer = timer_create_default();
LedControl lc=LedControl(36,40,26,1);
WebServer server(80);

bool Position = false;
bool Mo = true;
bool Map[8][8] = { 0 };
const static int shape_map[7][3][2] = {
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
const char PAGE_INDEX[] PROGMEM= R"=====(
<!DOCTYPE html>
<html>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>俄罗斯方块</title>
<body>
<script>
function right()
{select("r");}
function left()
{select("l");}
function rotate()
{ select("o");}
function select(message)
{
  xmlhttp=new XMLHttpRequest();
  var url =  "/?position=" + message;
  xmlhttp.open("GET",url,true);
  xmlhttp.send();
}
</script>
<div>
<center><button type="button" onclick="left()">left</button><button type="button" onclick="rotate()">rotate</button><button type="button" onclick="right()">right</button></center>
</div>
</body>
</html>
)=====";

void Draw();
bool judge_bound();
void Rotate();
bool AutoMove(void *);
bool judge_finish();
void right();
void left();
void Convert(int * new_x,int *new_y);
void handleRoot();
void game_over();
void summon();
void clear();
void init();

void setup() 
{
  Serial.begin(115200);
  lc.shutdown(0,false);
  lc.setIntensity(0,5);
  lc.clearDisplay(0);
  summon();
  WiFi.mode(WIFI_AP);
  WiFi.softAP("RC1", "");
  server.on("/", handleRoot);
  server.begin();
  timer.every(1000, AutoMove);
}

void loop() 
{
    server.handleClient();
    timer.tick();
}

bool judge_bound(int * x,int *y)
{
  for (int i =0 ; i < 4 ; i ++)
  {
    if (store_y[i]<0||y[i]<0) return true;
    if ( x[i] < 0|| x[i] > 7 || y[i] > 7)
    { return false; }
  }
  for (int i = 0; i < 4; i ++)
  {
      Map[store_y[i]][store_x[i]] = 0; 
  }
  for (int i = 0; i < 4; i ++)
  {
    if(Map[y[i]][x[i]] == 1)
    {
      for (int i = 0; i < 4; i ++)
          Map[store_y[i]][store_x[i]] = 1;
      return false;
    }
  }
  for (int i = 0; i < 4; i ++)
        Map[store_y[i]][store_x[i]] = 1;
  return true;
}

bool judge_bound(int * coordinate)
{
  for (int i = 0; i < 4 ; i ++)
  {
    if (coordinate[i]>7||coordinate[i]<0) return false;
  }
  for (int i = 0; i < 4 ; i ++)
  {
    if (store_y[i]<0) return true;
  }
  for (int i = 0; i < 4 ; i ++)
  {
    Map[store_y[i]][store_x[i]] = 0;
  }
  for (int i = 0; i < 4 ; i ++)
  {
    if(Map[store_y[i]][coordinate[i]]) 
    {
     for (int i = 0; i < 4 ; i ++)
      {
        Map[store_y[i]][store_x[i]] = 1;
      }
     return false; 
    }
  }
  for (int i = 0; i < 4 ; i ++)
  {
    Map[store_y[i]][store_x[i]] = 1;
  }
  return true;
}

bool judge_finish()
{
  server.handleClient();
  delay(100);
  for (int i = 0; i < 4 ; i ++)
  {
    if (store_y[i] == 7)
    {return true;}
  }
  for (int i = 0; i < 4 ; i ++)
    if (store_y[i]>=0)
      { Map[store_y[i]][store_x[i]] = 0; }
  for (int i = 0; i < 4; i ++ )
  {
    if (store_y[i]>=0&&Map[store_y[i]+1][store_x[i]])
    {
      for (int i = 0; i < 4; i ++ )
        if (store_y[i]>=0)
          Map[store_y[i]][store_x[i]] = 1;
      return true; 
    }
  }
  return false;
}

void summon()
{
  unsigned int shape = random(0,7);
  store_x[0] =  3;
  store_y[0] =  -1;
  store_x[1] =  3 + shape_map[shape][0][0];
  store_y[1] =  -1 + shape_map[shape][0][1];
  store_x[2] =  3 + shape_map[shape][1][0];
  store_y[2] =  -1 + shape_map[shape][1][1];
  store_x[3] =  3 + shape_map[shape][2][0];
  store_y[3] =  -1 + shape_map[shape][2][1];
  game_over();
  Clear();
}

void Clear()
{
  int i = 7;
  while (i >= 1)
  {
    for (int j = 0; j <= 7; j ++)
    {
      if (!Map[i][j])
      {
        i --;
        break;
      }
      else if (j == 7)
      {
        for (int m = i; m > 0; m --)
        {
          for (int n = 0; n <= 7; n ++)
             Map[m][n] = Map[m-1][n];
        }
      }
    }
  }
}

void game_over()
{
  for (int i = 0; i <=7 ;i ++)
  {
    if (Map[1][i] == 1) 
    {
      delay(500);
       lc.clearDisplay(0);
       lc. setColumn(0,4,B11111111);
       delay(1000);
       init();
    }
  }
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
      Convert(temp_x,temp_y);
      if(judge_finish())
      {
        summon();
      }
   }
}

void Move()
{
   Position = false;
  int temp_y[4];
  int temp_x[4];
  for (int i = 0; i < 4; i ++)
    temp_x[i] = store_x[i];
  for (int i = 0; i < 4; i ++)
  {
    temp_y[i] = store_y[i] + 1;
  }
  Convert(temp_x,temp_y);
  Position = true;
  server.handleClient();
  delay(300);
  if(judge_finish())
  {
    summon();
  }
}

bool AutoMove(void *)
{
  if (Mo)
    Move();
  return true;
}

void right()
{
  int temp_x[4],temp_y[4];
  for (int i = 0; i < 4; i ++)
    temp_y[i] = store_y[i];
  for (int i = 0; i < 4; i ++)
  {
    temp_x[i] = store_x[i] + 1;
  }
  if(judge_bound(temp_x))
  {
    Convert(temp_x,temp_y);
    if(judge_finish())
      {
        summon();
     }
  }
}

void left()
{
  int temp_x[4],temp_y[4];
  for (int i = 0; i < 4; i ++)
    temp_y[i] = store_y[i];
  for (int i = 0; i < 4; i ++)
  {
    temp_x[i] = store_x[i] - 1;
  }
  if(judge_bound(temp_x))
  {
    Convert(temp_x,temp_y);
    if(judge_finish())
      {
        summon();
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
      for (int i =0; i < 4; i ++)
      {
        store_x[i] = new_x[i];
        store_y[i] = new_y[i];
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

void init()
{
  for (int i = 0; i <= 7 ; i ++)
    for (int j = 0; j <= 7; j ++)
      Map[i][j] = 0;
  summon();
}

void handleRoot() 
{
  Mo = false;
  String web = PAGE_INDEX;
  server.send(200, "text/html", web);
  String message = server.arg("position");
  char temp[message.length()+1];
  strcpy(temp,message.c_str());
  if ((temp[0] =='r'||temp[0]=='l'||temp[0]=='o')&&Position) 
  {
     if (temp[0] == 'r') {right();Serial.println("r");}
     if (temp[0] == 'l') {left();Serial.println("l");}
     if (temp[0] == 'o') {Rotate();Serial.println("o");}
  }
    Mo = true;
    delay(100);
}
