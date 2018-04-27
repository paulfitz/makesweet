
#include "msmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PAULFITZ_MAXWIDTH 4000
#define PAULFITZ_MAXHEIGHT 4000
#define PAULFITZ_MAXLAYER 2

//#define DBG
//#define DISABLE_ME

int paulfitz_px, paulfitz_py, paulfitz_active = 0;
int paulfitz_max_layer = 0;
float paulfitz_xx[PAULFITZ_MAXLAYER][PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
float paulfitz_yy[PAULFITZ_MAXLAYER][PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
//float paulfitz_xx2[PAULFITZ_MAXLAYER][PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
//float paulfitz_yy2[PAULFITZ_MAXLAYER][PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
int paulfitz_ct[PAULFITZ_MAXLAYER][PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
char paulfitz_idx[PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
char paulfitz_ray[PAULFITZ_MAXWIDTH][PAULFITZ_MAXHEIGHT];
int paulfitz_remap[256];
int paulfitz_remapping = 0;
int paulfitz_setup = 0;
int paulfitz_aux = 0;
int paulfitz_bounce = 0;

int paulfitz_frame = 0;
int paulfitz_make_map = 0;

// Use a tiny private function to output images in the trivial ppm format.
// This avoids adding a dependency on another library.
static int SavePPM(char *src, const char *filename, int w, int h) {
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    printf("cannot open file %s for writing\n", filename);
    return -1;
  } else {
    fprintf(fp, "P6\n%d %d\n%d\n", w, h, 255);
    int i;
    for (i = 0; i < h; i++) {
      fwrite((void *) src, 1, (size_t) (w*3), fp);
      src += w*3;
    }
    
    fclose(fp);
  }
  return 0;
}

void msmap_render(int w, int h) {
  
  msmap_render_scaled(w,h,1);
}

void msmap_enable() {
  paulfitz_make_map = 1;
}

void msmap_set_frame(int frame) {
  paulfitz_frame = frame;
  paulfitz_setup = 0;
}

int msmap_parse_name(char *name) {
  if (name==NULL || (((long unsigned int) name)<256)) {
    static int message_shown = 0;
    if (!message_shown) {
      printf("msmap_parse_name: dodgy name! (%lu)\n", (long int)name);
      message_shown = 1;
    }
    return -1;
  }

  static const char *refname = "__makesweet_surface_n";
  static int reflen = 21; //strlen(refname);
  int idx = -1;

  if (name[0]=='_') {
    if (name[1]=='_') {
      if (name[2]=='m') {
	if (name[3]=='a') {
	  if (strlen(name)>reflen) {
	    idx = atoi(name+reflen);
	  }
	}
      }
    }
  }

 if (idx == -1) {
   static const char *refname = "mkswt_";
   static int reflen = 6; //strlen(refname);
   static char buf[256];
   if (name[0]=='m') {
     if (name[1]=='k') {
       if (name[2]=='s') {
	 if (name[3]=='w') {
	   if (strlen(name)>reflen) {
	     int at = 0;
	     int i = 0;
	     for (i=0; i<strlen(name); i++) {
	       char ch = name[i];
	       if (ch=='@') break;
	       if (ch>='0'&&ch<='9') {
		 buf[at] = ch;
		 if (at<200) at++;
	       }
	     }
	     buf[at] = '\0';
	     //idx = atoi(name+reflen)-1;
	     printf("From %s got %s\n", name, buf);
	     idx = atoi(buf)-1;
	     printf("GOT INDEX %d\n",idx);
	   }
	 }
       }
     }
   }
 }

#ifdef DBG
  printf("Parse %s as %d\n", name, idx);
#endif

  return idx;
}

void msmap_set_bounce(int bounce) {
  paulfitz_bounce = bounce;
}


void msmap_set_input(int idx, float fx, float fy) {
  //printf("Set input %d %g %g\n", idx, fx, fy);
  if (paulfitz_remapping) {
    idx = paulfitz_remap[idx];
    if (idx==-1) { 
      return;
    }
  }
  if (idx>paulfitz_max_layer) {
    paulfitz_max_layer = idx;
  }
#ifdef DISABLE_ME
  static int message_shown = 0;
  if (!message_shown) {
    printf("MSMAP invoked, but it is disabled so ignoring (msmap_set_input)\n");
    message_shown = 1;
  }
  return;
#endif
#ifdef DBG
  printf("Set input %d %g %g\n", idx, fx, fy);
#endif

  static int env_checked = 0;
  static int gave_warning = 0;
  if (!env_checked) {
    char *e = getenv("MAKESWEET_MAKE_MAP");
    if (e!=NULL) {
      paulfitz_make_map = (e[0]=='1')?1:0;
      printf("MAKESWEET_MAKE_MAP variable is %d\n", paulfitz_make_map);
    }
    env_checked = 1;
  }
  if (!paulfitz_make_map) {
#ifdef DBG
    printf("MAKESWEET_MAKE_MAP not set up, skipping\n");
#endif
    return;
  }

#ifdef DBG
  printf("idx is %d\n", idx);
#endif
  if (idx>=0) {
    if (!paulfitz_active) {
      if (gave_warning<10) {
	printf("WARNING: IMAGEWRAPOSA without active\n");
      } else if (gave_warning == 10) {
	printf("WARNING: last warning about IMAGEWRAPOSA without active\n");
      }
      gave_warning++;
    } else {
      if (!paulfitz_setup) {
	int i, j, k;
	printf("paulfitz setting up\n");
	for (i=0; i<PAULFITZ_MAXWIDTH; i++) {
	  for (j=0; j<PAULFITZ_MAXHEIGHT; j++) {
	    for (k=0; k<PAULFITZ_MAXLAYER; k++) {
	      paulfitz_xx[k][i][j] = 0;
	      paulfitz_yy[k][i][j] = 0;
	      //paulfitz_xx2[k][i][j] = 0;
	      //paulfitz_yy2[k][i][j] = 0;
	      paulfitz_ct[k][i][j] = 0;
	    }
	    paulfitz_idx[i][j] = 0;
	    paulfitz_ray[i][j] = 127;
	  }
	}
	printf("paulfitz set up\n");
	paulfitz_setup = 1;
      }
      float dist = 0.0;
      int ray = paulfitz_bounce;
      if (paulfitz_px>=0 && paulfitz_py>=0 &&
	  paulfitz_px<PAULFITZ_MAXWIDTH && 
	  paulfitz_py<PAULFITZ_MAXHEIGHT &&
	  idx<PAULFITZ_MAXLAYER) {
	if (ray<=paulfitz_ray[paulfitz_px][paulfitz_py]) {
	  if (ray<paulfitz_ray[paulfitz_px][paulfitz_py]) {
	    int k;
	    for (k=0; k<=paulfitz_max_layer; k++) {
	      paulfitz_xx[k][paulfitz_px][paulfitz_py] = 0;
	      paulfitz_yy[k][paulfitz_px][paulfitz_py] = 0;
	      paulfitz_ct[k][paulfitz_px][paulfitz_py] = 0;
	    }
	    paulfitz_ray[paulfitz_px][paulfitz_py] = ray;
	  }
	  paulfitz_xx[idx][paulfitz_px][paulfitz_py] += fx;
	  paulfitz_yy[idx][paulfitz_px][paulfitz_py] += fy;
	  //paulfitz_xx2[idx][paulfitz_px][paulfitz_py] += fx*fx;
	  //paulfitz_yy2[idx][paulfitz_px][paulfitz_py] += fy*fy;
	  paulfitz_ct[idx][paulfitz_px][paulfitz_py]++;
	}
      }
    }
  }
}



void msmap_set_output(int rx, int ry) {
#ifdef DBG
  printf("Set output %d %d\n", rx, ry);
#endif
  paulfitz_px = rx;
  paulfitz_py = ry;
  paulfitz_active = 1;
  paulfitz_bounce = 0;
}


void msmap_unset_output() {
  //paulfitz_px = 0;
  //paulfitz_py = 0;
  paulfitz_active = 0;
}

void msmap_set_aux(int aux) {
  paulfitz_aux = aux;
}


void msmap_render_scaled(int w, int h, int factor) {
#ifdef DBG
  printf("Makesweet Render %d %d\n", w, h);
#endif
  if (factor!=1) {
    printf("rescale is not implemented\n");
    exit(1);
  }
  if (!paulfitz_setup) {
    printf("MAKESWEET no output to write\n");
  } else {
    printf("MAKESWEET writing output %dx%d\n", w, h);
    int i, j;
    int ww = w/factor, hh = h/factor; // should be 4:3 aspect ratio
    int ww2 = w, hh2 = h;
    int dx = (ww2-ww)/2;  // now always 0
    int dy = (hh2-hh)/2;  // now always 0

    unsigned char *img = (unsigned char *) calloc(3,ww2*hh2);
    unsigned char *img2 = (unsigned char *) calloc(3,ww2*hh2);
    if (img!=NULL && img2!=NULL) {
      memset(img,255,3*ww2*hh2);
      memset(img2,255,3*ww2*hh2);
      for (i=0; i<ww; i++) {
	for (j=0; j<hh; j++) {
	  int d = 2;
	  if (!(i>=d&&j>=d&&i<ww-d&&j<hh-d)) {
	    d = 0;
	  }
	  int d2 = 1;
	  if (!(i>=d2&&j>=d2&&i<ww-d2&&j<hh-d2)) {
	    d2 = 0;
	  }
	  int max_ct = 0;
	  int tot_ct = 0;
	  //float best_sig = 1e9;
	  //float second_best_sig = 1e9;
	  //int winner_sig = -1;
	  int winner = -1;
	  int k, kx, ky;
	  /*
	  int cts[PAULFITZ_MAXLAYER];
	  //float txx[PAULFITZ_MAXLAYER];
	  //float txx2[PAULFITZ_MAXLAYER];
	  //float tyy[PAULFITZ_MAXLAYER];
	  //float tyy2[PAULFITZ_MAXLAYER];
	  //float sig2[PAULFITZ_MAXLAYER];
	  for (k=0; k<=paulfitz_max_layer; k++) {
	    cts[k] = 0;
	  }
	  for (kx=-d; kx<=d; kx++) {
	    for (ky=-d; ky<=d; ky++) {
	      for (k=0; k<=paulfitz_max_layer; k++) {
		cts[k] += paulfitz_ct[k][i+kx][j+ky];
		//txx[k] += paulfitz_xx[k][i+kx][j+ky];
		//txx2[k] += paulfitz_xx2[k][i+kx][j+ky];
		//tyy[k] += paulfitz_yy[k][i+kx][j+ky];
		//tyy2[k] += paulfitz_yy2[k][i+kx][j+ky];
	      }
	    }
	  }
	  */
	  for (k=0; k<=paulfitz_max_layer; k++) {
	    //int ct = cts[k];
	    int ct = paulfitz_ct[k][i][j];
	    //if (ct>0) {
	    //sig2[k] = txx2[k] - txx[k]*txx[k]  +  tyy2[k] - tyy[k]*tyy[k];
	    //}
	    //if (sig2[k]<best_sig) {
	    //second_best_sig = best_sig;
	    //best_sig = sig2[k];
	    //winner_sig = k;
	    //}
	    if (ct>max_ct) {
	      max_ct = ct;
	      winner = k;
	    }
	    tot_ct += ct;
	  }
	  //if (winner_sig>=0) {
	  //if (paulfitz_ct[winner_sig][i][j]>10) {
	  //winner = winner_sig;
	  //}
	  //}
	  float xx = 0;
	  float yy = 0;
	  int rct = 0;
	  if (tot_ct>0) {
	    if (max_ct>tot_ct*0.55) { // || best_sig<second_best_sig*0.8) {
	      paulfitz_idx[i][j] = winner+1;
	      /*
	      int c = 0;
	      int c2 = 0;
	      for (kx=-d2; kx<=d2; kx++) {
		for (ky=-d2; ky<=d2; ky++) {
		  int lct = paulfitz_ct[winner][i+kx][j+ky];
		  c++;
		  if (lct<1) {
		    if (kx!=0||ky!=0) continue;
		    lct = 1;
		  }
		  c2++;
		  xx += paulfitz_xx[winner][i+kx][j+ky]/lct;
		  yy += paulfitz_yy[winner][i+kx][j+ky]/lct;
		}
	      }
	      if (c!=c2) {
		int lct = paulfitz_ct[winner][i][j];
		if (lct<1) lct = 1;
		xx = paulfitz_xx[winner][i][j]/lct;
		yy = paulfitz_yy[winner][i][j]/lct;
	      } else {
		xx /= c;
		yy /= c;
	      }
	      */
	      int lct = paulfitz_ct[winner][i][j];
	      if (lct<1) lct = 1;
	      xx = paulfitz_xx[winner][i][j]/lct;
	      yy = paulfitz_yy[winner][i][j]/lct;
	      rct = 1;
	    } else {
	      paulfitz_idx[i][j] = -1; // mixed-up
	    }
	  }

	  int q1 = 255;
	  int q2 = 255;
	  int q3 = 255;
	  if (rct) {
	    int x = (int)(xx*4096+4096+0.5);
	    int y = (int)(yy*4096+4096+0.5);
	    if (x>4095) x -= 4096;
	    if (y>4095) y -= 4096;
	    if (x>4095) x = 4095;
	    if (y>4095) y = 4095;
	    if (x<0) x = 0;
	    if (y<0) y = 0;
	    y = 4095-y;
	    int p1 = x%256;
	    int p2 = x/256;
	    int p3 = y%256;
	    int p4 = y/256;
	    q1 = p1;
	    q2 = p3;
	    q3 = p2 + 16*p4;
	  }
	  if (i<ww && j<hh) {
	    int at = ((hh2-1-(dy+j))*ww+dx+i)*3;
	    img[at] = (unsigned char) q1;
	    img[at+1] = (unsigned char) q2;
	    img[at+2] = (unsigned char) q3;
	  }
	}
      }
      int maxn = paulfitz_max_layer + 1;
      int tricky = 0;
      for (i=0; i<ww; i++) {
	for (j=0; j<hh; j++) {
	  int idx = paulfitz_idx[i][j];
	  int q1 = 255;
	  int q2 = 255;
	  int q3 = 255;

	  if (idx>0) {
	    q1 = idx;
	    q2 = idx*64;
	    q3 = 0;
	  } else if (idx==-1) {
	    tricky = 1;
	  }
	  int at = ((hh2-1-(dy+j))*ww+dx+i)*3;
	  img2[at] = (unsigned char) q1;
	  img2[at+1] = (unsigned char) q2;
	  img2[at+2] = (unsigned char) q3;
	}
      }


      SavePPM((char *)img, "mapper.ppm", ww2, hh2);
      SavePPM((char *)img2, "mapper2.ppm", ww2, hh2);

      FILE *fout = fopen("mapper.txt","w");
      if (fout==NULL) {
	printf("cannot write to mapper.txt");
	exit(1);
      }
      fprintf(fout,"maxn=%d\n", maxn);
      fprintf(fout,"tricky=%d\n", tricky);

      printf("PAULFITZ wrote mapper.ppm mapper2.ppm mapper.txt\n");
      free(img);
      free(img2);
      img = NULL;
    }
  }

  paulfitz_setup = 0;
}


int msmap_set_index(int external_index, int internal_index) {
  int i;
  if (!paulfitz_remapping) {
    paulfitz_remapping = 1;
    for (i=0; i<256; i++) {
      paulfitz_remap[i] = -1;
    }
  }
  paulfitz_remap[internal_index] = external_index;
  return 0;
}
