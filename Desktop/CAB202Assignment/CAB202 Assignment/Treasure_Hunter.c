#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <ncurses.h>

#define DELAY (10)
#define HERO_WIDTH (2)
#define HERO_HEIGHT (3)
#define MAX_BLOCK_WIDTH (7)
#define MAX_BLOCK_HEIGHT (5)
#define MAX_PLATFORMS (200)
#define MAX_FORBIDDEN_PLATFORMS (40)


//Game state
bool game_over = false;
bool update_screen = true;
bool falling = false;


char * treasure_sprite =
" == "
"|**|";

char * player_sprite =
" o "
"-|-"
"< >";

char *forbiddenPlatform =
"xxxxxx "
"xxxxxx ";

char *safePlatform =
"====== "
"====== ";


sprite_id platforms[MAX_PLATFORMS];
sprite_id forbiddenPlatforms[MAX_FORBIDDEN_PLATFORMS];
int lengthOfForbiddenPlatforms = 0;
int lengthOfAll = 0;
int respawnX = 0;
int respawnY = 0;
int lives = 10;


void end_game(){
  game_over = true;
}

// map generation
void map_generator(int numOfRows, int numOfColumns, char *map[numOfRows][numOfColumns])
{
  int maxNumPlatforms = 2 * numOfColumns;

  // round 1 generate safe spots one safe platform per column
  for (int col = 0; col < numOfColumns; ++col)
  {
    int rx = col;
    int ry = rand() % numOfRows;
    map[ry][rx] = "=";
  }

  maxNumPlatforms -= numOfColumns; // take out the safe blocks we placed 
  if(numOfColumns >= 4)
  {
    int forbiddenToPlace = (numOfColumns / 4);
    // round 2 place forbidden block to catch up to the rule of 1 forbidden for every 4 safe
    for (int i = 0; i < forbiddenToPlace; ++i)
    {
      int rx = rand() % numOfColumns;
      int ry = rand() % numOfRows;

      if (strcmp(map[ry][rx], "=") == 0)
      {
        i -= 1; 
      } else {
        map[ry][rx] = "x";
      }
    }
    maxNumPlatforms -= forbiddenToPlace;
  }

  //round 3 generate the rest of the platforms 
  int safeCount = 0;
  for (int i = 0; i < maxNumPlatforms; ++i)
  {
    char *platform = "=";
    safeCount += 1;
    if (safeCount == 4)
    {
      safeCount = 0;
      platform = "x";
    }
    int rx = rand() % numOfColumns;
    int ry = rand() % numOfRows;
    if (strcmp(map[ry][rx], "=") == 0)
      {
        i -= 1; 
      } else if (strcmp(map[ry][rx], "x") == 0)
      {
        i -= 1;
      }else {
        map[ry][rx] = platform;
      }
  }
}

void setup_map(int numOfRows, int numOfColumns, char *map[numOfRows][numOfColumns])
{
	for (int row = 0; row < numOfRows; ++row)
	{
		for (int col = 0; col < numOfColumns; ++col)
		{
      int y = (row * MAX_BLOCK_HEIGHT) + (5 + MAX_BLOCK_HEIGHT);
      int x = col * MAX_BLOCK_WIDTH;
      if (strcmp(map[row][col],"=") == 0)
      {
        sprite_id platform_id = sprite_create(x, y, 7, 2, safePlatform);

        platforms[lengthOfAll] = platform_id;
        ++lengthOfAll;

      } else if (strcmp(map[row][col], "x") == 0){
        sprite_id platform_id = sprite_create(x, y, 7, 2, forbiddenPlatform);

        platforms[lengthOfAll] = platform_id;
        forbiddenPlatforms[lengthOfForbiddenPlatforms] = platform_id;
        ++lengthOfForbiddenPlatforms;
        ++lengthOfAll;
      }
		}
	}
}

void draw_map()
{
	for (int id = 0; id < lengthOfAll; ++id)
	{
		sprite_draw(platforms[id]);
	}
}

void draw_background(void)
{
  draw_line(0,0,0,screen_height()-1,'|');                                //Draws border right
  draw_line(screen_width()-1,0,screen_width()-1,screen_height()-1,'|');  //Draws border left
  draw_line(0,0,screen_width()-1,0, '-');
  draw_line(0,4, screen_width()-1, 4, '-');
}


void display_time(double startTime)
{
  int gameTime = (int)(get_current_time() - startTime); // Get current time - start time to get time running
  int mins = (int) gameTime / 60;                       // Gets the hrs
  int secs = (int) gameTime % 60;                       // Gets the seconds '%'(modulo) is like division but i gives you the remainder

  char timeString[] = "Time:";
  char timeFormatter[] = "%02d:%02d";                   // We use this to format the time with a pading of one 0 infront if the digit is less than 10
  char timeFormatted[6];                                // The character array to store the formatted time string in
  sprintf(timeFormatted,timeFormatter,mins,secs);
  draw_string(5, 2, timeString);
  draw_string(11,2, "     ");
  draw_string(11,2, timeFormatted);
}


// for testing
void output_map(int numOfRows, int numOfColumns, char *map[numOfRows][numOfColumns])
{
  FILE *f = fopen("map.txt","w");

  for (int row = 0; row < numOfRows; ++row)
  {
    for (int col = 0; col < numOfColumns; ++col)
    {
      printf("%s ", map[row][col]);
      fprintf(f, "%s ", map[row][col]);
    }
    printf("\n");
    fprintf(f,"\n");
  }
  int forbiddenPlatforms = 0;
  int safePlatforms = 0;
  int holes = 0;

  for (int row = 0; row < numOfRows; ++row)
  {
    for (int col = 0; col < numOfColumns; ++col)
    {
      if (strcmp(map[row][col], "=") == 0)
      {
        safePlatforms += 1;
      } else if (strcmp(map[row][col], "x") == 0)
      {
        forbiddenPlatforms += 1;
      } else{
        holes += 1;
      }
    }
  }
  fprintf(f, "Screen Height: %i\n", screen_height());
  fprintf(f, "Screen Width: %i\n", screen_width());
  fprintf(f, "Columns: %i\n", numOfColumns);
  fprintf(f, "Rows: %i\n", numOfRows);
  fprintf(f, "Max Platforms: %i\n", 2 * numOfColumns);
  fprintf(f, "Forbidden: %i\n", forbiddenPlatforms);
  fprintf(f, "Safe: %i\n", safePlatforms);
  fprintf(f, "Holes: %i\n", holes);
  fprintf(f, "Length Of All: %i\n", lengthOfAll);
  fprintf(f, "Length Of Forbidden Platforms: %i\n", lengthOfForbiddenPlatforms);
  fclose(f);
}


bool check_collisions_individual(sprite_id a, sprite_id b)
{
  int left1 = round( sprite_x( a ) );
  int left2 = round( sprite_x( b ) );
  int top1 = round( sprite_y( a ) );
  int top2 = round( sprite_y( b ) );
  int right1 = left1 + sprite_width( a ) - 1;
  int right2 = left2 + sprite_width( b ) - 2;
  int bottom1 = top1 + sprite_height( a );
  int bottom2 = top2 + sprite_height( b ) - 1;

  if ( left1 > right2 )
      return false;
  if ( left2 > right1 )
      return false;
  if ( top1 > bottom2 )
      return false;
  if ( top2 > bottom1)
      return false;

  return true;
}

void check_unit_Collision(sprite_id player_id)
{
  int player = round(sprite_x(player_id));
  int leftSide = 0;
  int rightSide = screen_width();
  int bottom = screen_height();
  if (player < leftSide)
  {
    sprite_init(player_id, respawnX, respawnY, 3, 3, player_sprite);
    lives -= 1;
  }
  else if (player > rightSide)
  {
    sprite_init(player_id, respawnX, respawnY, 3, 3, player_sprite);
    lives -= 1;
  }
  else if (player > bottom)
  {
    sprite_init(player_id, respawnX, respawnY, 3, 3, player_sprite);
    lives -= 1;
  }

}
bool check_collisions(sprite_id player_id)
{
  bool isFalling;
  // iterate through and first check if we have died
  for (int i = 0; i < lengthOfForbiddenPlatforms; ++i)
  {
    if (check_collisions_individual(player_id, forbiddenPlatforms[i])){
      lives -= 1;
      if (lives == 0){
        end_game();
      }
      sprite_init(player_id,respawnX, respawnY, 3, 3, player_sprite);
    } 
  }

  // then iterate through and see if we have hit platform
  for (int i = 0; i < lengthOfAll; ++i)
  {
    if (check_collisions_individual(player_id, platforms[i]))
    {
      return false;
    }
  }
  return true;
}

void move_player(int Hdirection, int yDirection, bool activateJump, sprite_id player_id)
{
  static int jumpCount = 0;
  static bool jumpActive = false;
  if (activateJump)
  {
    jumpActive = true;
    jumpCount = 0;
  }

  if (jumpCount <= 3 && jumpActive){
    yDirection = -1;
    jumpCount += 1;
  }else{
    jumpCount = yDirection;
    jumpActive = false;
  }
  sprite_move(player_id,Hdirection,yDirection);
}


sprite_id spawn_player(int numOfColumns, int numOfRows,  char *map[numOfRows][numOfColumns] )
{
  for (int col = 0; col < numOfColumns; ++col){
    if(strcmp(map[0][col],"=") == 0){
      int ySpawn = 7;
      int xSpawn = col * MAX_BLOCK_WIDTH;
      sprite_id player_id = sprite_create(xSpawn,ySpawn, 3, 3, player_sprite);
      return player_id;
    }
  }
  return 0;
}

sprite_id spawn_treasure()
{
  int ySpawn = screen_height() - 2;
  int xSpawn = rand() % screen_width() - 4;
      sprite_id treasure_id = sprite_create(xSpawn,ySpawn, 4, 2, treasure_sprite);
      return treasure_id;
}


void gameloop(double startTime, sprite_id player_id, sprite_id treasure_id)
{
  bool isFalling = check_collisions(player_id);
  int jumpOrFall = 0;
  int key; 
  int Hdirection = 0;
  bool activateJump = false;           
  key = getch();        // Get key pressed returns key pressed in decimal
  if (key == 113) {     // If 'q' is pressed we will quit 113 is the decimal for 'q' use http://www.asciitable.com/
    game_over = true;
  }

  // input movement matrix
  if(isFalling){
    jumpOrFall += 1;
  }else{
    jumpOrFall = 0;
  }

  if(key == 97){                  // a key
    Hdirection = -1;
  }else if (key == 100){          // d key
    Hdirection = 1;
  }else if (key == 119){
    activateJump = true;
  }

  move_player(Hdirection,jumpOrFall, activateJump, player_id);
  check_unit_Collision(player_id);
  if (check_collisions_individual(player_id, treasure_id))
  {
    lives += 1;
    sprite_init(player_id, respawnX, respawnY, 3, 3, player_sprite);
  }
  if (lives == 0)
  {
    end_game();
  }
  draw_string(20, 2, "Lives:");
    draw_int(26,2, lives);
    draw_string(30, 2, "n10026151");
    show_screen();
 
  timer_pause(10);
}

void graphics_loop(sprite_id player_id, sprite_id treasure_id, double startTime){
  clear_screen();
  draw_background();
  draw_map();
  sprite_draw(player_id);
  sprite_draw(treasure_id);
  display_time(startTime);
  show_screen();
}


int main()
{
  lengthOfForbiddenPlatforms = 0;
  lengthOfAll = 0;

	srand(time(NULL));
	setup_screen();
	
  int numOfRows = ((screen_height() - 5) / MAX_BLOCK_HEIGHT) - 1;
  int numOfColumns =  screen_width() / MAX_BLOCK_WIDTH;
  char *map[numOfRows][numOfColumns];

	for (int x = 0; x < numOfRows; ++x)
		{
  		for (int y = 0; y < numOfColumns; ++y)
  		{
    			map[x][y] = " ";                         // default variable for the array
  		}
		}
	map_generator(numOfRows, numOfColumns, map);
  setup_map(numOfRows,numOfColumns, map);          // creates the sprites and return length of the platform list.
  output_map(numOfRows,numOfColumns,map);          // for testing  
	draw_string(0, screen_height() -1, "Press q to finish...");
	double startTime = get_current_time();
  sprite_id player_id = spawn_player(numOfColumns, numOfRows, map);
  sprite_id treasure_id = spawn_treasure();
  respawnX = sprite_x(player_id);
  respawnY = sprite_y(player_id);
	while (!game_over) {
  	gameloop(startTime, player_id, treasure_id);
    graphics_loop(player_id,treasure_id , startTime);
	}
	cleanup_screen();
	return 0;
}
