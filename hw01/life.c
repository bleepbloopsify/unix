#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEFAULT_SIZE 10
#define DEFAULT_NUM_GENERATIONS 10
#define START_GENERATION 0
#define ALIVE '*'
#define DEAD  '-'

typedef struct {
  size_t alive;
  size_t neighbors;
} Cell;

// Cells are [| | | | | |]
// horizontal first, columns second. so (x, y) indexing
typedef struct {
  Cell** cells;
  size_t rows, cols;
  size_t generations;
} Board;

Cell createCell() {
  Cell cell;
  cell.alive = 0; // every cell starts dead.
  // start them at 0 by default. assuming that calculating neighbors is first
  cell.neighbors = 0; 
  return cell;
}

// Rows go -->
// Cols go |
//         |
//         v
// So rows are indexed by x
// cols are indexed by y
Board createBoard(const unsigned int rows, const unsigned int cols) {
  Board board;
  unsigned int x, y;

  board.rows = rows;
  board.cols = cols;
  // this is is in big caps so we can change it easily if we want
  board.generations = START_GENERATION;

  board.cells = malloc(sizeof(Cell*) * cols);

  for (x = 0; x < cols; ++x) {
    board.cells[x] = malloc(sizeof(Cell) * rows);

    for (y = 0; y < rows; ++y) {
      board.cells[x][y] = createCell();
    }
  }

  return board;
}

// cleanup after use
void freeBoard(const Board* board) {
  unsigned int x;

  for (x = 0; x < board->rows; ++x) {
    free(board->cells[x]);
  }

  free(board->cells);
}

// load in from file
void loadWorld(const Board* board, const char* fname) {
  FILE* fp;
  char* line = NULL;
  ssize_t nread;
  size_t linelen;
  char c;
  unsigned int x, y;
  Cell* cell;

  fp = fopen(fname, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open file %s\n", fname);
    exit(1);
  }

  for (y = 0; y < board->rows; ++y) {
    // reassign here in case something becomes realloc'd by getline
    linelen = board->cols; 
    nread = getline(&line, &linelen, fp);
    if (nread == -1) { // eof or other err. we just stop loading at this point
      break;
    }
    for (x = 0; x < nread && x < board->rows; ++x) {
      c = line[x];

      cell = &board->cells[x][y];
      cell->alive = (c == ALIVE);
    }
  }

  free(line);
  fclose(fp);
}

// generate using time seed.
void generateWorld(const Board* board) {
  unsigned int x, y;
  Cell* cell;

  srand(time(0));

  for (x = 0; x < board->cols; ++x) {
    for(y = 0; y < board->rows; ++y) {
      cell = &board->cells[x][y];

      cell->alive = rand() % 2;
    }
  }
}

// basic display
void displayBoard(const Board board) {
  printf("Generation %lu\n", board.generations);
  unsigned int x, y;

  for (y = 0; y < board.rows; ++y) {
    for (x = 0; x < board.cols; ++x) {
      if (board.cells[x][y].alive) {
        printf("%c", ALIVE);
      } else {
        printf("%c", DEAD);
      }
    }

    printf("\n");
  }
  
  printf("================================\n");
}

// utility to mark number of neighbors for each cell.
void updateNeighbors(const Board* board) {
  unsigned int x, y;
  int xoff, yoff;
  Cell* cell;

  for (x = 0; x < board->cols; ++x) {
    for (y = 0; y < board->rows; ++y) {
      cell = &board->cells[x][y];

      cell->neighbors = 0; // start it at 0;

      for (xoff = -1; xoff < 2; ++xoff) {
        for (yoff = -1; yoff < 2; ++yoff) {
          if (x + xoff < 0 || x + xoff >= board->cols) continue;
          if (y + yoff < 0 || y + yoff >= board->rows) continue; 


          cell->neighbors += board->cells[x + xoff][y + yoff].alive;
        }
      }

      cell->neighbors -= cell->alive;
    }
  }
}

// big function to mark neighbors as well as updating cells
void nextGeneration(Board* board) {
  unsigned int x, y;
  Cell* cell;

  updateNeighbors(board);

  for (x = 0; x < board->cols; ++x) {
    for (y = 0; y < board->rows; ++y) {
      
      cell = &board->cells[x][y];

      if(cell->alive && (cell->neighbors < 2 || cell->neighbors > 3)) {
        cell->alive = 0;
      } else if (!cell->alive && cell->neighbors == 3) {
        cell->alive = 1;
      }
    }
  }

  board->generations += 1;
}

int main(const int argc, const char** argv) {
  int i;
  unsigned int rows = DEFAULT_SIZE, cols = DEFAULT_SIZE;
  unsigned int generations = DEFAULT_NUM_GENERATIONS;
  FILE* file;
  Board board;

  if (argc > 1) { // have rows
    rows = strtoul(argv[1], NULL, 10); // expect rows to be number
  }
  if (argc > 2) { // have cols
    cols = strtoul(argv[2], NULL, 10); // same as above
  }

  // we initialize like this. all cells are initialized as dead.
  board = createBoard(rows, cols);

  if (argc > 3) { // have filename!
    loadWorld(&board, argv[3]); // load it in
  } else {
    loadWorld(&board, "life.txt");
    // apparently life.txt is supposed to be default 
    //generateWorld(&board); // meh just make a random world
    // wrote generateWorld before i read the assignment spec properly
  }

  if (argc > 4) { // woot
    generations = strtoul(argv[4], NULL, 10); // lets go forever!
  }

  displayBoard(board); // show it once for gen 0
  while (board.generations < generations) { // go go go
    nextGeneration(&board);
    displayBoard(board);
  } // ok done


  freeBoard(&board); // kill everything
  return 0; // we are a-ok
}
