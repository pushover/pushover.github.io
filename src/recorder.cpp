#include "recorder.h"

#include <stdio.h>

void recorder_c::load(const std::string & filename) {
  FILE * f = fopen(filename.c_str(), "r");

  static char ll[200];
  fgets(ll, 200, f);

  // remove the newline at the end
  ll[strlen(ll)-1] = 0;
  levelName = ll;

  playpos = 0;

  record.clear();
  while (!feof(f))
  {
    int cnt, val;
    fscanf(f, "%i %i\n", &cnt, &val);
    for (int i = 0; i < cnt; i++)
      record.push_back(val);
  }
}

void recorder_c::save(void) const {
  int num = 0;
  FILE * f = 0;

  do {
    num++;

    if (f) fclose(f);

    char fname[200];

    snprintf(fname, 200, "recordings/%05i.rec", num);
    f = fopen(fname, "r");

  } while (f);

  char fname[200];

  snprintf(fname, 200, "recordings/%05i.rec", num);
  f = fopen(fname, "w");

  fprintf(f, "%s\n", levelName.c_str());

  int val = record[0];
  int cnt = 1;
  unsigned int pos = 1;

  while (pos < record.size()) {

    if (record[pos] != val) {
      fprintf(f, "%i %i\n", cnt, val);
      val = record[pos];
      cnt = 1;
    }
    else
    {
      cnt++;
    }
    pos++;
  }

  fprintf(f, "%i %i\n", cnt, val);
  fclose(f);
}


