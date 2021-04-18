/* definitions for the CHS clock card 
 */
 
 
/*
 * The definitions of registers within the MSM5832 chip
 */
#define CRTC_REGISTER_UNITS_OF_SECONDS      (0)
#define CRTC_REGISTER_TENS_OF_SECONDS       (1)
#define CRTC_REGISTER_UNITS_OF_MINUTES      (2)
#define CRTC_REGISTER_TENS_OF_MINUTES       (3)
#define CRTC_REGISTER_UNITS_OF_HOURS        (4)
#define CRTC_REGISTER_TENS_OF_HOURS         (5)
#define CRTC_REGISTER_DAY_OF_WEEK           (6)
#define CRTC_REGISTER_UNITS_OF_DAYS         (7)
#define CRTC_REGISTER_TENS_OF_DAYS          (8)
#define CRTC_REGISTER_UNITS_OF_MONTHS       (9)
#define CRTC_REGISTER_TENS_OF_MONTHS        (10)
#define CRTC_REGISTER_UNITS_OF_YEAR         (11)
#define CRTC_REGISTER_TENS_OF_YEAR          (12)

/*
 * Mode control - bits within the tens of hours and tens of days register to give operating mode 
 */
#define CRTC_HOURS_MODE_24H                 (0x08)               /* Set for 24 hour mode */
#define CRTC_HOURS_MODE_PM                  (0x04)               /* Set for PM */
#define CRTC_DAYS_MODE_LEAP_YEAR            (0x04)               /* Set for leap year */


/*
 * Bit defintions within the control register 
 */
#define CRTC_CONTROL_ADDRESS_MASK           (0x0f)               /* The address bits fed to the rtc       */
#define CRTC_CONTROL_READ                   (0x10)               /* Read strobe to the rtc (Active High)  */
#define CRTC_CONTROL_WRITE                  (0x20)               /* Read strobe to the rtc (Active High)  */
#define CRTC_CONTROL_HOLD                   (0x40)               /* Hold signal to the rtc (Active High)  */
#define CRTC_CONTROL_ADJUST                 (0x80)               /* Adjust signal to the rtc (Active High)*/


extern int crtc_PIOportadata_read( int portaddress);
extern void crtc_PIOportadata_write( int portaddress, int port_data);
extern int crtc_PIOportbdata_read( int portaddress);
extern void crtc_PIOportbdata_write( int portaddress, int port_data);

extern int crtc_PIOportacontrol_read( int portaddress );
extern void crtc_PIOportacontrol_write( int portaddress, int port_data);
extern int crtc_PIOportbcontrol_read( int portaddress );
extern void crtc_PIOportbcontrol_write( int portaddress, int port_data);
