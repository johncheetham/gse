/*

    gse 0.1.6  May 2012

    Copyright (C) 2010, 2011, 2012 John Cheetham    
   
    web   : http://www.johncheetham.com/projects/gshogi/#gse
    email : developer@johncheetham.com

    gse is a USI Shogi engine. gse is short for GShogi Engine.
    gse is based on gShogi (it is a USI version of the gshogi engine).
    gShogi was based on GNU Shogi. 
     
    gse.c - This module is the USI interface between the gui and the
            engine.    
   
    This file is part of gse    

    gse is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gse is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with gse.  If not, see <http://www.gnu.org/licenses/>.
   
 */

#define _GNU_SOURCE
#include "version.h"
#include "gnushogi.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>   

short goinfinite = false;
char bestmove[6];
FILE *logfile = NULL;
short uselog = false;


void zprintf ( char * format, ... )
{
    va_list ap;
    char *p, *sval;
    char buf[256];
    char *q;

    q = buf;
    va_start(ap, format);
    for (p = format; *p; p++)
    {
        if (*p != '%')
        {
            *q = *p;
            q++;
            continue;
        }
        switch (*++p)
        {
        case 's':
            for (sval = va_arg(ap, char *); *sval; sval++)
            {
                *q = *sval;
                q++;
            }
            break;
        default:
            *q = *p;
            q++;
            break;
        }        
    }
    va_end(ap);
    *q = '\0';

    printf(buf);
    fflush(stdout);

    if (uselog)           
    {        
        fprintf(logfile, "%s%s", "<", buf);        
        fflush(logfile);
    }
}


/* computer move */
void *engine_cmove(void *arg)
{
    bestmove[0] = '\0';    
    winner = 0;
    compptr = (compptr + 1) % MINGAMEIN;    
     
    if (!(flag.quit || flag.mate || flag.force))
    {               
        SelectMove(computer, FOREGROUND_MODE);        

        if (computer == white)
        {
            if (flag.gamein)
            {
                TimeCalc();
            }
            else if (TimeControl.moves[computer] == 0)
            {
                if (XC)
                {
                    if (XCmore < XC)
                    {
                        TCmoves = XCmoves[XCmore];
                        TCminutes = XCminutes[XCmore];
                        TCseconds = XCseconds[XCmore];
                        XCmore++;
                    }
                }
                 SetTimeControl();
            }
        }
    }
    if (!goinfinite)
    {        
        zprintf("bestmove %s\n", mvstr[0]);           
    }
    else
    {
        /* go infinite ended early */
        /* don't pass back bestmove until stop command received */ 
        strcpy(bestmove, mvstr[0]);
    }    
    pthread_exit(NULL);        
}


/* human move */
void engine_hmove(char *hmove)
{  
    short rc;   
    winner = 0;
    oppptr = (oppptr + 1) % MINGAMEIN;    
    rc = DoMove(hmove); /* in commondsp.c */                
}


/* set computer player to black or white*/
void set_player(short plyr)
{    
    if (plyr == black)
    {
        player = black;
        opponent = black;
        computer = white;                  
    }
    else
    {
        player = white;
        opponent = white;
        computer = black;             
    }
}


/* set computer player to black or white*/
void set_computer_player(short plyr)
{    
    if (plyr == black)
    {
        player = white;
        opponent = white;
        computer = black;           
    }
    else
    {
        player = black;
        opponent = black;
        computer = white;       
    }
}


int get_val(char *cmd, char *str)
{
    int val = 0;
    char *p;
    
    if ( (p = strstr(cmd, str)) )
    {
        p += strlen(str);
        val = atoi(p);    
    }
    return val;
    
}


/* convert a time in milliseconds to a string in the format MM:SS */
void convert_time(char *mmss, int time_ms)
{
    int mins, secs;
    char smins[3];
    char ssecs[3];    

    secs = time_ms / 1000;
    mins = secs / 60;
    if (mins > 59)
    {
        mins = 59;
        secs = 59;
    }
    else if (mins > 0)
    {
        secs = secs % (mins * 60);
    }

    sprintf(smins, "%02d", mins);
    sprintf(ssecs, "%02d", secs);    
    
    strcpy(mmss, smins);
    strcat(mmss, ":");
    strcat(mmss, ssecs);    
}


void
set_fen(char *fen)
{

    /*                                                                                       */
    /* USI SFEN string                                                                       */
    /*                                                                                       */
    /* uppercase letters for black pieces, lowercase letters for white pieces                */
    /*                                                                                       */
    /* examples:                                                                             */
    /*      lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1                  */
    /*      8l/1l+R2P3/p2pBG1pp/kps1p4/Nn1P2G2/P1P1P2PP/1PS6/1KSG3+r1/LN2+p3L w Sbgn3p 124   */ 

    /*

   values in engine

   board[l]
   0 - unoccupied
   1 - Pawn            (p)
   2 - Lance           (l)
   3 - Knight          (n)
   4 - Silver General  (s)
   5 - Gold General    (g)
   6 - Bishop          (b)
   7 - Rook            (r)
  14 - King            (k) 


   colour
   0 - black
   1 - white
   2 - neutral
  
   promoted
   0 - not promoted
   1 - promoted
  
  Board used in gse

                       ROW
    L N S G K G S N L   8     l = 72..80
    - R - - - - - B -   7     l = 63..71
    P P P P P P P P P   6     l = 54..62
    - - - - - - - - -   5     l = 45..53
    - - - - - - - - -   4     l = 36..44
    - - - - - - - - -   3     l = 27..35
    p p p p p p p p p   2     l = 18..26
    - b - - - - - r -   1     l =  9..17
    l n s g k g s n l   0     l =  0..8  

COL 0 1 2 3 4 5 6 7 8  

*/
    char *p;    
    int c, i, j, z;
    short sq;
    short side, isp;
    char ch;
    char num[3];

    p = fen;    
    NewGame();
    flag.regularstart = false;
    Book = false;

    i = NO_ROWS - 1; /* i is row index */
    j = 0; /* j is column index */

    while (*p != ' ') 
    {
        if (*p == '/')
        {
            j = 0;
            i--;
            p++;
        }
        
        if (*p == '+')
        {
            isp = 1;
            p++;
        }
        else
        {
            isp = 0;
        }        

        sq = i * NO_COLS + j;

        if (isdigit(*p))
        {
            num[0] = *p;
            num[1] = '\0';            
            for (z = 0; z < atoi(num); z++)
            {                
                board[sq] = no_piece;
                color[sq] = neutral;
                j++;
                sq = i * NO_COLS + j;                
            }
            j--;            
        }
        else
        {
            for (c = 0; c < NO_PIECES; c++)
            {
                if (*p == qxx[c])
                {
                    if (isp)
                    {
                        board[sq] = promoted[c];                        
                    }
                    else
                    {
                        board[sq] = unpromoted[c];                        
                    }

                    color[sq] = white;
                }
            }

            for (c = 0; c < NO_PIECES; c++)
            {
                if (*p == pxx[c])
                {
                    if (isp)
                    {
                        board[sq] = promoted[c];                        
                    }    
                    else
                    {
                        board[sq] = unpromoted[c];                        
                    }

                    color[sq] = black;
                }
            }
        }
        p++;
        j++;
    }

    /* side to move */
    p++;
    if (*p == 'w')
    {
        set_player(white);
    }
    else if (*p == 'b')
    {
        set_player(black);
    }
    else
    {
        fprintf(stderr, "error - side to move not valid in sfen: %c\n",*p);
        set_computer_player(black);
    }    
    
    p++;
    if (*p != ' ')
    {
        fprintf(stderr, "error in sfen string. no space after side to move\n");
    }

    /* pieces in hand */
    ClearCaptured();
    p++;
    if (*p != '-')
    {
        while (*p != ' '  && *p != '\n' && *p != '\0')
        {        
            strcpy(num, "1");
            if (isdigit(*p))
            {
                num[0] = *p;
                num[1] = '\0';
                p++;
                if (isdigit(*p))
                {
                    num[1] = *p;
                    num[2] = '\0';
                    p++;
                }
            
            }

            ch = *p;
            if isupper(ch)
            {
                side = black;
                ch = tolower(ch);
            }
            else
            {
                side = white;
            }

        
            if (ch == 'p')
            {
                Captured[side][pawn]   = atoi(num);
            }
            else if (ch == 'l')
            {
                Captured[side][lance]  = atoi(num);
            }
            else if (ch == 'n')
            {
                Captured[side][knight] = atoi(num);
            }
            else if (ch == 's')
            {
                Captured[side][silver] = atoi(num);
            }
            else if (ch == 'g')
            {
                Captured[side][gold]   = atoi(num);
            }
            else if (ch == 'b')
            {
                Captured[side][bishop] = atoi(num);
            }
            else if (ch == 'r')
            {
                Captured[side][rook]   = atoi(num);
            }
            p++;
        }
        
    }
   
    Game50 = 1;
    ZeroRPT();
    InitializeStats();
    UpdateDisplay(0, 0, 1, 0);
    Sdepth = 0;
    hint = 0;
}


void get_settings(char LogFilename[], short *uselog)
{
    FILE *settings_file = NULL;
    char uselogfile[10];
   
    /* default settings */
    strcpy(LogFilename, "logfile.log");
    *uselog = false;

    /* Try and restore settings from settings file */
    if ((settings_file = fopen("settings", "r")) == NULL)
    {
        return;
    }

    if (fgets(LogFilename, 256, settings_file) == NULL)
    {
        return;
    }    

    /* lose the \n */
    LogFilename[strlen(LogFilename) - 1] = '\0';

    if (fgets(uselogfile, sizeof(uselogfile), settings_file) == NULL)
    {
        return;
    }   

    if (strncmp(uselogfile, "true", strlen("true")) == 0)
    {       
        *uselog = true;
    }
    else
    {
        *uselog = false;
    }      
    fclose(settings_file);    
}


void save_settings(char LogFilename[], short uselog)
{
    FILE *settings_file = NULL;

    if ((settings_file = fopen("settings", "w")) == NULL)
    {
        return;
    }

    fprintf(settings_file, "%s\n", LogFilename);

    if (uselog)
    {
        fprintf(settings_file, "%s\n", "true");
    }
    else
    {
        fprintf(settings_file, "%s\n", "false");
    }

    fclose(settings_file);
}


int main(int argc, char *argv[])
{    
    char *cmd; 
    char *p;
    char test_str[100];    
    char level[20];
    char mmss[6]; 
    char gse_version[10] = "0.1.6";    
    char LogFilename[256];    
   
    int bytes_read;    
    size_t nbytes = 100;
      
    char m[11];

    pthread_t cthread;    
    
    int i;    
    int rc;
    char *end;

    int byoyomi, movetime = 0;        
    
    /* list args*/
    /*
    printf("argc = %d\n", argc);
    for (i = 0; i < argc; i++)
        printf("argv[%d] = %s\n", i, argv[i]); 
    */       
    
    if (argc > 1)
    {            
        /* if call is to create opening book then do it and exit */
        if (strcmp(argv[1], "-g") == 0)
        {        
            if (argc != 6)
            {
                printf("invalid call to create opening book\n");
                return 0;
            }       
            bookfile = (char *) malloc (strlen(argv[2]) + 1);
            strcpy(bookfile, argv[2]);
        
            binbookfile = (char *) malloc (strlen(argv[3] + 1));
            strcpy(binbookfile, argv[3]);        

            booksize = atoi(argv[4]);
            bookmaxply = atoi(argv[5]);         
        
            InitMain();
            GetOpenings();
            return 0; 
        }
        else if (strcmp(argv[1], "--version") == 0)     
        {
            printf("gse %s\n", gse_version);  
            printf("Copyright (C) 2010 John Cheetham\n"); 
            printf("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n");  
            printf("This is free software: you are free to change and redistribute it.\n"); 
            printf("There is NO WARRANTY, to the extent permitted by law.\n"); 
            return 0;
        }        
    }
    
    /* get settings */
    get_settings(LogFilename, &uselog);    

    binbookfile = (char *) malloc (strlen("gse.bbk") + 1);
    strcpy(binbookfile, "gse.bbk");  

    cmd = (char *) malloc (nbytes + 1);
    bytes_read = getline (&cmd, &nbytes, stdin);     
   
    rc = InitMain();    

    if (uselog)
    {       
        logfile = fopen(LogFilename,"w");        
    }

    while (strncmp(cmd, "quit", strlen("quit")) != 0)
    {
        if (uselog)           
        {
            fprintf(logfile, ">%s", cmd);
            fflush(logfile);
        }
        
        if (strncmp(cmd, "usinewgame", strlen("usinewgame")) == 0)
        {          
            NewGame();
        }
        else if (strncmp(cmd, "usi", strlen("usi")) == 0)
        {                   
            zprintf("id name gse %s\n", gse_version);
            zprintf("id author John Cheetham\n");           

            zprintf("option name Logfile type string default %s\n", LogFilename);
            zprintf("option name UseLog type check default %s\n", uselog ? "true" : "false");

            zprintf("usiok\n");             
        }
        else if (strncmp(cmd, "isready", strlen("isready")) == 0)
        {
            zprintf("readyok\n");             
        }        
        /* position startpos moves 7g7f */
        else if (strncmp(cmd, "position", strlen("position")) == 0)
        {                        
            if (strncmp(cmd, "position startpos", strlen("position startpos")) == 0)            
            {                
                NewGame();                             
                set_player(black);                
                strcpy(test_str, "position startpos moves ");
                if (strncmp(cmd, test_str, strlen(test_str)) == 0) 
                {
                    p = cmd + strlen(test_str);                
                    end = p + strlen(p);

                    while (p < end)                
                    {                    
                        i = 0;
                        while (p[0] != ' ' && p[0] != '\n' && p[0] != '\0')
                        {                        
                            m[i] = p[0];
                            i++;
                            p++;
                        }
                        p++;
                        m[i] = '\0';                    
                        engine_hmove(m);
                        /* if moves are repeated 3 times then gse will assume a draw */
                        /* and flag.mate will get set to true. We do not want this here */
                        flag.mate = false;                        
                        if (player == black)
                        {
                            set_player(white);                           
                        }
                        else
                        {
                            set_player(black);                           
                        }
                    }
                }
            }
            else if (strncmp(cmd, "position sfen", strlen("position sfen")) == 0)
            {
                p = cmd + strlen("position sfen ");
                set_fen(p);
                p = strstr(cmd, " moves ");
                if (p)
                {
                    p = p + strlen(" moves ");                
                    end = p + strlen(p);

                    while (p < end)                
                    {                    
                        i = 0;
                        while (p[0] != ' ' && p[0] != '\n' && p[0] != '\0')
                        {                        
                            m[i] = p[0];
                            i++;
                            p++;
                        }
                        p++;
                        m[i] = '\0';                                 
                        engine_hmove(m);
                        /* if moves are repeated 3 times then gse will assume a draw */
                        /* and flag.mate will get set to true. We do not want this here */
                        flag.mate = false;
                        if (player == black)
                        {                            
                            set_player(white);
                        }
                        else
                        {                            
                            set_player(black);
                        }
                    }
                }                
            }
        }
        /* go */
        else if (strncmp(cmd, "go", strlen("go")) == 0)
        {
            goinfinite = false;
            set_computer_player(player);            
            if (winner == (opponent + 1))
            {
                zprintf("bestmove resign\n");                
            }
            else
            {
                /* set defaults */
                MaxSearchDepth = 39;
                NodeCntLimit = 0;
                strcpy(level, "level 0 59:59");
                InputCommand(level);                

                if (strstr(cmd, "depth"))                
                {
                    MaxSearchDepth = (short)get_val(cmd, "depth");                                      
                }

                if (strstr(cmd, "nodes"))                
                {
                    NodeCntLimit = (long)get_val(cmd, "nodes");                                      
                }

                if (strstr(cmd, "infinite"))                
                {
                    /* go infinite */
                    goinfinite = true;                    
                }

                if (strstr(cmd, "btime") || strstr(cmd, "wtime") || strstr(cmd, "byoyomi") || strstr(cmd, "movetime"))  
                {
                    int inc, moves;
                    inc = 0;
                    

                    /* example go commands */

                    /* classical - e.g. 5 minutes for each 40 moves */
                    /* go btime 300000 wtime 300000 movestogo 40 */

                    /* incremental - e.g. 5 minutes plus 10 seconds per move */
                    /* go btime 300000 wtime 300000 binc 10000 winc 10000 */

                    /* fixed time - e.g. 10 seconds per move. time for move is in movetime */
                    /* go movetime 10000 */

                    /* fixed search depth of 10 */
                    /* go depth 10 */

                    /* byoyomi - not USI compliant */
                    /* go btime 36000000 wtime 36000000 byoyomi 60000 */


                    movetime = get_val(cmd, "movetime");
                    moves = get_val(cmd, "movestogo");
                    if (movetime > 0 && moves == 0) 
                    {
                        moves = 1;
                    }                    

                    if (movetime == 0)
                    {
                        if (computer == white)
                        {                
                            movetime = get_val(cmd, "wtime");                
                            inc = get_val(cmd, "winc");                
                        }
                        else
                        {                
                            movetime = get_val(cmd, "btime");
                            inc = get_val(cmd, "binc");                
                        }
                    }                
            
                    byoyomi = get_val(cmd, "byoyomi");                    

                    if (movetime > 0)
                    {
                        movetime = movetime / (moves ? moves : 80) + inc;                      
                    }
                    else if (byoyomi > movetime)
                    {
                        movetime = byoyomi;                
                    }
                    else
                    /* no time left - set to 1 second*/
                    {
                        movetime = 1000;                 
                    }                    

                    /* if less than 1 second set to 1 second */ 
                    if (movetime < 1000)
                    {
                        movetime = 1000;
                    }

                    convert_time(mmss, movetime);            

                    strcpy(level, "level 0 ");
                    strcat(level, mmss);            
            
                    InputCommand(level);                               
                }

                if (uselog)
                {
                    fprintf(logfile, "#\n");
                    fprintf(logfile, "# depth set to %d\n", MaxSearchDepth); 
                    fprintf(logfile, "# nodes set to %ld", NodeCntLimit);
                    if (NodeCntLimit > 0)
                    {
                        fprintf(logfile, "\n");
                    }
                    else
                    {
                        fprintf(logfile, " (no limit)\n");
                    }
                    fprintf(logfile, "# level set to %s\n", level);
                    fprintf(logfile, "#\n");
                    fflush(logfile); 
                }               

                rc = pthread_create(&cthread, NULL, engine_cmove, NULL);
                if (rc)
                {
                    printf("ERROR; return code from pthread_create() is %d\n", rc);
                    exit(-1);
                }
            }            
        }
        else if (strncmp(cmd, "stop", strlen("stop")) == 0)
        {
            goinfinite = false;
            /* if a 'go infinite' search ended early then move is in bestmove */
            if (bestmove[0] != '\0')
            {
                zprintf("bestmove %s\n", bestmove);                 
            }
            else
            {
                /* stop the search and return the bestmove */
                flag.musttimeout = true;
            }
            
        }
        else if (strncmp(cmd, "setoption", strlen("setoption")) == 0)
        {                  
            strcpy(test_str, "setoption name Logfile value ");                            
            if (strncmp(cmd, test_str, strlen(test_str)) == 0)
            {
                if (strlen(cmd + strlen(test_str)) < sizeof(LogFilename));
                {
                    strcpy(LogFilename, cmd + strlen(test_str));
                    LogFilename[strlen(LogFilename) - 1] = '\0';
                }
                  
            }
            
            strcpy(test_str, "setoption name UseLog value ");
            if (strncmp(cmd, test_str, strlen(test_str)) == 0)            
            {
                if (strncmp(cmd + strlen(test_str), "true", strlen("true")) == 0)
                {
                    /* open log file */                    
                    if (logfile == NULL)
                    {
                        logfile = fopen(LogFilename,"w");
                    }

                    /* check if open was successful */                
                    if (logfile == NULL)
                    {
                        uselog = false;
                    }       
                    else                    
                    {
                        uselog = true;
                    }                         
                }
                else
                {
                    uselog = false;
                    /* if logfile is open then close it */
                    if (logfile != NULL)
                    {
                        fclose(logfile);
                    }
                }
            }           
        }
        bytes_read = getline (&cmd, &nbytes, stdin);
    }

    free(cmd);

    /* get settings */
    save_settings(LogFilename, uselog);  

    if (logfile != NULL)
    {
        fclose(logfile);
    }

    return 0;
}

