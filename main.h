
void IERROR (char * msg);
void IPRINT (char * msg);

// #define __GUARDING

#ifdef __GUARDING
  #define __GUARD(A,N) \
    printf("**ALLC;%p;%s\n", A, N);
  #define __UNGUARD(A,N) \
    printf("**FREE;%p;%s\n", A, N);
#else
  #define __GUARD(A,N) \
    ;
  #define __UNGUARD(A,N) \
    ;
#endif


#define PI 3.1415
#define PI2 6.2831
#define ANG0 0
#define ANG1 0.01745277777777778
#define ANG15 0.2617916666666667
#define ANG30 0.5235833333333334
#define ANGN30 5.759416666666667
#define ANG45 0.785375
#define ANG60 1.0471666666666668
#define ANG90 1.57075
#define ANGN90 4.71225
#define ANG120 2.0943951
#define ANG180 3.1415
#define ANG360 6.283

/*
  IPC Protocol

  target  code  purpose

     666    21  kill      (p_game)
     666    5x  load
     666    6x  save
     666    9x  theme

       1     0  intro     (p_cutscene)
       1     1  death
       1     2  win
       1     3  credits
*/
