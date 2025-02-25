/*******************************************************************
*
* ntp_rtc_lcd_clock.c
*
* NTP Clock for RPi PICO-W
*
* Uses HD44780 16x2 LCD with I2C interface
*
* NTPv4 specification: https://www.rfc-editor.org/rfc/rfc5905
*
********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "pico/cyw43_arch.h"

#include "hardware/rtc.h"
#include "hardware/i2c.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "hd44780_lcd_api.h"

#include "bsttimes.h"

#define NTP_SERVER_ADDR "uk.pool.ntp.org"
#define NTP_PORT 123
#define NTP_MESSAGE_LEN 48

// British Summer Time 1 hour offset
#define BST_OFFSET (60 * 60) 

// NTP uses an epoch of 1 January 1900. Unix uses an epoch of 1 January 1970. 
#define NTP_EPOCH_OFFSET 2208988800

static char *dayofweek[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char *timezone[2] = { "GMT", "BST" };

static ip_addr_t ntp_server_address;
static bool dns_request_sent;

static struct udp_pcb *udp_pcb = NULL; //UDP protocol control block

static int current_day = 0;
static bool dst = false;

/******************************************************************
*
* dst_check()
*
* BST is last Sunday in March to last Sunday in October
*
* check current time is within bst_start_times & bst_end_times for BST
*
*
*******************************************************************/
static bool dst_check( const time_t unix_format_time, const int year )
{
    bool bst;   
    if ( (unix_format_time > bst_start_times[year-BST_START_YEAR] ) && (unix_format_time < bst_end_times[year-BST_START_YEAR] ) )
        bst = true;
    else
        bst = false;             

   return bst;
}
/******************************************************************
*
* ntp_request()
*
* send UDP NTP request message
*
*******************************************************************/
static void ntp_request( void ) 
{
    struct pbuf *pbuf;
    uint8_t *req;

    cyw43_arch_lwip_begin();

    pbuf = pbuf_alloc(PBUF_TRANSPORT, NTP_MESSAGE_LEN, PBUF_RAM);

    req = (uint8_t *) pbuf->payload;

    memset(req, 0, NTP_MESSAGE_LEN);
    
    // NTP request:  0x1B or 00 011 011 means
    // LI   = 0    (Leap indicator)
    // VN   = 3    (Version number)
    // Mode = 3    (Mode, mode 3 is client mode)
    req[0] = 0x1B;
    
    udp_sendto( udp_pcb, pbuf, &ntp_server_address, NTP_PORT );

    pbuf_free( pbuf );

    cyw43_arch_lwip_end();
}

/******************************************************************
*
* ntp_dns_found()
*
* callback for dns_gethostbyname()
*
*******************************************************************/
static void ntp_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) 
{   
    if (ipaddr) 
    {
        ntp_server_address = *ipaddr;
        printf("found ntp address %s\n", ipaddr_ntoa(ipaddr));
        ntp_request();
    } 
    else 
    {
        printf("ntp dns request failed\n");
        dns_request_sent = false;
    }
}


/******************************************************************
*
* ntp_receive()
*
* Callback for udp_recv() with NTP data received
* update RTC with received NTP time
*
*******************************************************************/
static void ntp_receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) 
{
    if ( ip_addr_cmp( addr, &ntp_server_address ) && ( port == NTP_PORT ) && ( p->tot_len == NTP_MESSAGE_LEN ) ) 
    {
        uint8_t mode;
        uint8_t stratum;

        mode = pbuf_get_at( p, 0 ) & 0x7;
        stratum = pbuf_get_at( p, 1 ); 
        // check mode = 0x4 (server) + stratum != 0 (valid)        
        if ( ( mode == 0x4 ) && ( stratum != 0 ) )
        {
            uint32_t ntp_seconds;
            uint32_t unix_seconds;
            time_t unix_epoch;
            datetime_t t;
            struct tm *ntp_tm;

            //NTP timestamp bytes 40-44 network unsigned long format
            ntp_seconds = pbuf_get_at( p, 40 ) << 24 | 
                          pbuf_get_at( p, 41 ) << 16 | 
                          pbuf_get_at( p, 42 ) << 8 | 
                          pbuf_get_at( p, 43 );
                          
            //NTP epoch 1900 => Unix epoch 1970
            unix_seconds = ntp_seconds - NTP_EPOCH_OFFSET;
            unix_epoch = unix_seconds;
        
            ntp_tm = gmtime( &unix_epoch );

            if ( dst_check( unix_epoch, 1900+ntp_tm->tm_year) )
            {
                unix_epoch += BST_OFFSET;
                ntp_tm = gmtime( &unix_epoch );
            }

            printf("NTP RX: %s\n", asctime(ntp_tm) );

            tm_to_datetime( ntp_tm, &t );
    
            rtc_set_datetime( &t );
        }
    } 
    else 
    {
        printf("invalid ntp response\n");
    }

    dns_request_sent = false;
    pbuf_free(p);
}

/******************************************************************
*
* ntp_get_time()
*
* Connect to Wi-Fi SSID
* Request NTP time from NTP server
*
*******************************************************************/
static int ntp_get_time( void )
{
    int retval=0;
    
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) 
    {
        printf("failed to connect to %s\n", WIFI_SSID);
        retval = -1;
    }
    else
    {
        udp_pcb = udp_new_ip_type( IPADDR_TYPE_ANY );
        if ( udp_pcb == NULL ) 
        {
            printf("failed to create udp pcb\n");
            retval = -1;
        }
        else
        {
            int err;
            udp_recv( udp_pcb, ntp_receive, NULL );

            cyw43_arch_lwip_begin();

            err = dns_gethostbyname( NTP_SERVER_ADDR, &ntp_server_address, ntp_dns_found, NULL );

            cyw43_arch_lwip_end();

            dns_request_sent = true;
    
            if (err == ERR_OK) 
            {
                ntp_request( );
            } 
            else if (err == ERR_INPROGRESS)
            {
                while ( dns_request_sent )
                {
                    sleep_ms(1000);
                }
            }
            else if (err != ERR_INPROGRESS) 
            {
                printf("dns request failed\n");
                dns_request_sent = false;
                retval = -1;
            }
        }
        udp_remove( udp_pcb );
    }
    
    cyw43_arch_disable_sta_mode();
    
    return retval;
}

/******************************************************************
*
* main()
*
* Main Program Body
*
*******************************************************************/
int main() 
{      
    setup_default_uart();

    printf("\n\n\nLCD Test: main()\n");
    
    /* PICO-W I2C0 on the default SDA and SCL pins (4, 5) 400KHz I2C */
    i2c_init( PICO_DEFAULT_I2C_INSTANCE(), 400000 );
    gpio_set_function( PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C );
    gpio_set_function( PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C );
    gpio_pull_up( PICO_DEFAULT_I2C_SDA_PIN );
    gpio_pull_up( PICO_DEFAULT_I2C_SCL_PIN );

    /* Initialize LCD */
    hd44780_lcd_init();

    /* Initialize RTC */
    rtc_init();

    /* Initialize Wi-Fi */
    if ( cyw43_arch_init() ) 
    {
        printf("cyw43_arch failed to initialise\n");
    }
    else
    {          
        hd44780_lcd_clear();
        hd44780_lcd_set_cursor(0, 0);
        hd44780_lcd_string( "===NTP Clock===" ); 
           
        ntp_get_time();
               
        while (true) 
        {           
            datetime_t t;
            time_t tsecs;
            char datetime_buf[256];
            
            rtc_get_datetime( &t );
            
            sprintf( datetime_buf, "%s %02d %s %04d", dayofweek[t.dotw], t.day, months[t.month-1], t.year);
            printf("\r%s  ", datetime_buf);
            hd44780_lcd_set_cursor(0, 0);
            hd44780_lcd_string( datetime_buf );

            sprintf( datetime_buf, "%02d:%02d:%02d    %03s", t.hour, t.min, t.sec, dst_check( tsecs, t.year) ? timezone[1]:timezone[0] );
            printf("   %s", datetime_buf);
            hd44780_lcd_set_cursor(1, 0);
            hd44780_lcd_string( datetime_buf );

            /* update NTP time at 3am */
            if ( ( t.day != current_day ) && ( t.hour == 3 ) )
            {
                ntp_get_time();
                current_day = t.day;
            }
            sleep_ms(1000);
        }
    }
}
