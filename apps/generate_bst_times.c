/********************************************************
* generate_bst_times.c
*
* Calculate British Summer Time (BST) start & end times
* for period of years
*
* BST starts 2am last Sunday in March
* BST ends 2am last Sunday in October
*********************************************************/
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>
#include <time.h>

static int debug = 0;

static char *dayofweek[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/********************************************************
* calculate_bst_times()
*
* Calculate British Summer Time (BST) start & end times 
* for year
*
* BST starts 2am last Sunday in March
* BST ends 2am last Sunday in October
*********************************************************/
static void calculate_bst_times( const int year, time_t *bst_start_time, time_t *bst_end_time )
{
    if ( !bst_start_time || !bst_end_time )
    {
        printf("%s: invalid arguments\n", __FUNCTION__);
    }
    else
    {
        time_t tsecs;
        struct tm bst_tm, *tm, *bst_start_tm, *bst_end_tm;
        uint32_t sunday_offset;
    
        bst_tm.tm_year = year-1900;
        bst_tm.tm_mon = 2; /* March */
        bst_tm.tm_mday = 31;
        bst_tm.tm_hour = 0;
        bst_tm.tm_min = 0;
        bst_tm.tm_sec = 0;
    
        tsecs = mktime( &bst_tm );
        tm = gmtime( &tsecs ); 
        if (debug)
           printf( "31st March = %s\n", dayofweek[tm->tm_wday] );
        // adjust date to last Sunday on month
        sunday_offset = ( tm->tm_wday % 7) * (24 * 60 * 60);   
        if (debug)
            printf("sunday offset = %u\n", sunday_offset );
        *bst_start_time = (time_t)(tsecs - sunday_offset);
        if (debug)
            printf("bst_start_time=%u\n", (unsigned int)*bst_start_time );
        bst_start_tm = gmtime( bst_start_time );
        if (debug)
        {
            printf("bst_start_tm=%s\n", asctime( bst_start_tm ) );
            printf( "BST Start Date: %s %d %s %04d %02d:%02d\n", 
                 dayofweek[bst_start_tm->tm_wday], bst_start_tm->tm_mday, months[bst_start_tm->tm_mon], bst_start_tm->tm_year+1900, 
                 bst_start_tm->tm_hour, bst_start_tm->tm_min); 
        }
        bst_tm.tm_year = year-1900;
        bst_tm.tm_mon = 9; /* October */
        bst_tm.tm_mday = 31;
        bst_tm.tm_hour = 0;
        bst_tm.tm_min = 0;
        bst_tm.tm_sec = 0;
        
        tsecs = mktime( &bst_tm );
        tm = gmtime( &tsecs ); 
        if (debug)
            printf( "31st October = %s\n", dayofweek[tm->tm_wday] );
        // adjust date to last Sunday on month
        sunday_offset = ( tm->tm_wday % 7) * (24 * 60 * 60);   
        if (debug)
            printf("sunday offset = %u\n", sunday_offset );
        *bst_end_time = (time_t)(tsecs - sunday_offset);
        if (debug)
           printf("bst_end_time=%u\n", (unsigned int)*bst_end_time );
        bst_end_tm = gmtime( bst_end_time );
        if (debug)
        {
            printf("bst_end_tm=%s\n", asctime( bst_end_tm ) );
            printf( "BST End Date: %s %d %s %04d %02d:%02d\n", 
                 dayofweek[bst_end_tm->tm_wday], bst_end_tm->tm_mday, months[bst_end_tm->tm_mon], bst_end_tm->tm_year+1900, 
                 bst_end_tm->tm_hour, bst_end_tm->tm_min);            
        }
    }
}

/********************************************************
* main()
*
* main program body
*
* Calculate British Summer Time (BST) start & end times 
* for period of years and output as C header file format
*
*********************************************************/
int main( int argc, char *argv[] )
{
    time_t *bst_start_times = NULL;
    time_t *bst_end_times = NULL;
    if ( argc < 3 )
    {
        printf("Usage: bst_times <year> <count>\n");
    }
    else
    {
        int start_year, year_count, i;
        
        sscanf( argv[1], "%d", &start_year );
        sscanf( argv[2], "%d", &year_count );
        printf("#define BST_START_YEAR %d\n", start_year);
        printf("#define BST_NUM_YEARS  %d\n", year_count );
        
        bst_start_times = (time_t *)malloc( year_count*sizeof(time_t) );
        bst_end_times = (time_t *)malloc( year_count*sizeof(time_t) );
       
        if ( bst_start_times && bst_end_times )
        {
            struct tm *bst_start_tm, *bst_end_tm;

            /* generate BST start/end times */          
            for ( i = 0; i < year_count; i++)
            {
                calculate_bst_times( start_year+i, &bst_start_times[i], &bst_end_times[i] );
            }

            /* output BST start times */
            printf("\n/* ");
            for ( i = 0; i < year_count-1; i++)
            {
                bst_start_tm = gmtime( &bst_start_times[i] );
                printf( "%s %d %s %04d, ", 
             dayofweek[bst_start_tm->tm_wday], bst_start_tm->tm_mday, months[bst_start_tm->tm_mon], bst_start_tm->tm_year+1900 );
            } 
            bst_start_tm = gmtime( &bst_start_times[year_count-1] );
            printf( "%s %d %s %04d */\n", 
             dayofweek[bst_start_tm->tm_wday], bst_start_tm->tm_mday, months[bst_start_tm->tm_mon], bst_start_tm->tm_year+1900 );
            
            printf("const uint32_t bst_start_times[BST_NUM_YEARS] =  { ");
            for ( i = 0; i < year_count-1; i++)
            {
                printf( "%u, ", (unsigned int)bst_start_times[i] );
            } 
            printf( "%u ", (unsigned int)bst_start_times[year_count-1] );      
            printf("};\n");
        
            printf("\n/* ");
            
            /* output BST end times */
            for ( i = 0; i < year_count-1; i++)
            {
                bst_end_tm = gmtime( &bst_end_times[i] );
                printf( "%s %d %s %04d, ", 
             dayofweek[bst_end_tm->tm_wday], bst_end_tm->tm_mday, months[bst_end_tm->tm_mon], bst_end_tm->tm_year+1900 );
            } 
            bst_end_tm = gmtime( &bst_end_times[year_count-1] );
            printf( "%s %d %s %04d */\n", 
             dayofweek[bst_end_tm->tm_wday], bst_end_tm->tm_mday, months[bst_end_tm->tm_mon], bst_end_tm->tm_year+1900 );
             
            printf("const uint32_t bst_end_times[BST_NUM_YEARS] =  { ");
            for ( i = 0; i < year_count-1; i++)
            {
                printf( "%u, ", (unsigned int)bst_end_times[i] );
            } 
            printf( "%u ", (unsigned int)bst_end_times[year_count-1] );      
            printf("};\n");
        }
        free( bst_start_times );
        free( bst_end_times );
    }
    return 0;
}
