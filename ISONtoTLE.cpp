#include "utility.h"

#include <ctype.h>
#include <string.h> // strtok, strcpy stuff not even used yet
#include <iostream>
#include <stdlib.h> // atoi, atof not used yet
#include <math.h>  // math functions
#include <time.h>

#define MU  398600.4418  // gravitational parameter
#define MIN_PER_DAY  24 * 60  // minutes per day
#define SEC_PER_DAY  (MIN_PER_DAY * 60)   //seconds per day
#define TLE_LINE_LENGTH 80

using namespace std;

int calculateChecksum(char *line) 
{
    int check = 0;
    int i = 0;
    char a;
    for( i = 0; i != strlen(line) -1; ++i) 
    {
        a = line[i];
        if(isdigit(a))
            check += a - '0';
        if(line[i] == '-')
            check++;
    }
    return(check % 10);
}

float orbital_properties(float semimajor_axis, float e)
{
    float earth_radius = 6378.135;
    float mu = 398600.4418;
    float seconds_per_day = 86400.0;
    float minutes_per_day = seconds_per_day / 60.0;

    float apogee = (semimajor_axis * (1 + e)) - earth_radius;
    float perigee = (semimajor_axis * (1 - e)) - earth_radius;
    float period = 2.0 * M_PI * sqrt(pow(semimajor_axis, 3) / mu);
    float mean_motion = seconds_per_day / period;

    return (mean_motion);
}
class Tle
{
    public:
        char platform[25];
        char line1[TLE_LINE_LENGTH];
        char line2[TLE_LINE_LENGTH];

        int satnumber ;
        char classification[2];
        int id_launch_year ;
        int id_launch_number ;
        char id_launch_piece[4];
        int epoch_year ;
        float epoch_day ;
        char epoch[50] ;
        float mean_motion_derivative ;
        float mean_motion_sec_derivative ;
        float bstar ;
        int ephemeris_type ;
        int element_number ;
        float inclination ;
        float right_ascension ;
        float eccentricity ;
        float arg_perigee ;
        float mean_anomaly ;
        float mean_motion ;
        int orbit ;
        bool linesGood;
        //calculated parameters
        float semi_major;
        float apogee;
        float perigee;
    
    float tleStrToFloat(char * in){
        char temp[10];
        if(in[0] == '-') temp[0] = '-';
        else temp[0] = ' ';
        int offset = 1;
        if(isdigit(in[0])) offset = 0;
        temp[1] = '.';
        for (int i = 0; i < 4; i++){
            temp[2+i] = in[offset+i];
        };
        temp[6] = 'e';
        temp[7] = in[offset+5];
        temp[8] = in[offset+6];
        temp[9] = '\0';
        float b = atof(temp);
        return(b);
    }        
    Tle(){}; // empty constructor
    Tle(char *satName, char * l1, char* l2) {
        strncpy(platform, satName, sizeof(platform));
        strncpy(line1, l1, sizeof(line1));
        strncpy(line2, l2, sizeof(line2));
        linesGood = checksum(line1) & checksum(line2);
        int linenum;
        char mm2d[10], bstr[10], temp[12];
        sscanf( line1, "%d %5d%1s %2d%3d%3s %2d%f %f %s %s %d %4d",
                    &linenum,
                    &satnumber, classification,
                    &id_launch_year, &id_launch_number, id_launch_piece,
                    &epoch_year, &epoch_day, 
                    &mean_motion_derivative,
                    mm2d,
                    bstr,
                    &ephemeris_type,
                    &element_number); // scan card #1

        sscanf( line2, "%d %5d %f %f %f %f %f %11f%5d",
                    &linenum,
                    &satnumber,
                    &inclination,
                    &right_ascension,
                    &eccentricity,
                    &arg_perigee,
                    &mean_anomaly,
                    &mean_motion,
                    &orbit); // scan card #1
        // now fix the assumed decimal points
        mean_motion_sec_derivative = tleStrToFloat(mm2d);
        bstar = tleStrToFloat(bstr);
/*//this block assumes values are in a std::string
        if (mm2d[0] == '-') mm2d.insert(mm2d.begin()+1, '.'); //leading - sign
        else(!isdigit(mm2d[0])) mm2d[0] = '.'; //leading blank or + sign
        //else mm2d.insert(mm2d.begin(), '.'); //digit in first character
        mm2d.insert(mm2d.end() -2, 'e');
        mean_motion_sec_derivative = atof(mm2d.c_str());
 
         if (bstr[0] == '-') bstr.insert(bstr.begin()+1, '.'); //leading - sign
        else if (!isdigit(bstr[0])) bstr[0] = '.'; //leading blank or + sign
        else bstr.insert(bstr.begin(), '.'); //digit in first character
        bstr.insert(bstr.end() -2, 'e');
        bstar = atof(bstr.c_str());
 */
       
        // eccentricity = eccentricity / 1e7;
        
        // float tmp = pow((SEC_PER_DAY / (mean_motion * 2 * 3.141592653589)), 2);
        // semi_major = pow(MU, .33333333) * pow(tmp, .33333333);
        // perigee = semi_major * (1 - eccentricity) - 6378.135;
        // apogee = semi_major * (1 + eccentricity) - 6378.135;
    }
    
    bool checksum(char *line) 
    {
        int check = 0;
        int i = 0;
        char a;
        for( i = 0; i != strlen(line) -1; ++i) 
        {
            a = line[i];
            if(isdigit(a))
                check += a - '0';
            if(line[i] == '-')
                check++;
        }
        a = line[strlen(line) - 1]; // -1 for 0 based another -1 to exclude the checksum
        if((check % 10) != a - '0')
            return(false);
        else
            return(true);
    }
    
    void print(FILE* out = stdout) { //if no file given prints to stdout (i.e. terminal)
        fprintf(out, "Name: %s\n", platform);
        fprintf(out, "satno: %d\n", satnumber );
        fprintf(out, "inclination: %f\n", inclination);
        fprintf(out, "right_ascension: %f\n", right_ascension );
        fprintf(out, "eccentricity: %f\n", eccentricity );
        fprintf(out, "arg_perigee: %f\n", arg_perigee );
        fprintf(out, "mean_anomaly: %f\n", mean_anomaly );
        fprintf(out, "mean_motion: %11.8f\n", mean_motion );
        fprintf(out, "semi_major axis: %f\n", semi_major );
        fprintf(out, "apogee: %f\n", apogee );
        fprintf(out, "perigee: %f\n", perigee );
        fprintf(out, "orbit: %d\n", orbit);
//        fprintf(stdout, "", );
    }
    
    void printTLEFormat(FILE* out = stdout){
        char tempString[70];
        int checksum = 0;
        //TLE Line 0, title line
        fprintf(out, "%s", platform);
        //TLE Line 1
        sprintf(tempString, "1 %5.5u%1.1s %2.2u%3.3u%-3.3s %2.2u%012.8f 0.00000000  00000-0  00000-0 0    0",
            satnumber, classification, id_launch_year, id_launch_number, id_launch_piece, 
            epoch_year, epoch_day);
        checksum = calculateChecksum(tempString);
        fprintf(out,"%s%1u\n", tempString, checksum);
        //TLE Line 2
        sprintf(tempString, "2 %5.5u %8.4f %8.4f %07.0f %8.4f %8.4f %11.8f    0",
            satnumber, inclination, right_ascension, eccentricity, arg_perigee,
            mean_anomaly, mean_motion);
        checksum = calculateChecksum(tempString);
        fprintf(out,"%s%1u\n", tempString, checksum);

    }
};

class Vimpel
{
    public:
        std::string vimpel_line;
        int    sequence;
        int    so_num;
        int  date_1st_measurement;
        int reference_date;
        int  reference_tod;
        int    gap;
        float  major_semi_axis;
        float  inclination;
        float  right_ascension;
        float  eccentricity;
        float  latitude_at_reference;
        float  perigee;
        float  area_mass_ratio;
        float  magnitude;
        float  time_uncertainty;
        int    pos_uncertainty;
    
        // float  apogee;   // calculated later
        // float  perigee;  // calculated later
        Vimpel(char* line)
        {
            vimpel_line = std::string(line);
            sscanf(line, "%d, %d,%d, %d %d, %d, %f, %f,%f,%f,%f,%f,%f,%f, %f, %d",
               &sequence, //%d
               &so_num, //%d
               &date_1st_measurement, //%d
               &reference_date, //%d
               &reference_tod, //%d
               &gap,
               &major_semi_axis,
               &inclination,
               &right_ascension,
               &eccentricity,
               &latitude_at_reference,
               &perigee,
               &area_mass_ratio,
               &magnitude,
               &time_uncertainty,
               &pos_uncertainty
            );
        }

        void print(FILE* out = stdout)
        {
            fprintf(out, "sequence: %d\n", sequence);
            fprintf(out, "so_num: %d\n", so_num);
            fprintf(out, "date_1st: %d\n", date_1st_measurement);
            fprintf(out,"reference date: %d\n", reference_date);
            fprintf(out,"reference TOD: %d\n", reference_tod);
            fprintf(out,"gap: %d\n",gap);
            fprintf(out,"major semi axis: %3.1f\n",major_semi_axis);
            fprintf(out,"inclination: %3.3f\n", inclination);
            fprintf(out,"right ascention: %3.3f\n", right_ascension);
            fprintf(out,"eccentricity: %1.6f\n", eccentricity);
            fprintf(out,"latitude at reference: %3.1f\n", latitude_at_reference);
            fprintf(out,"perigee: %3.3f\n", perigee);
            fprintf(out,"area_mass_ratio: %1.2e\n", area_mass_ratio);
            fprintf(out,"magnitude: %2.1f\n", magnitude);
            fprintf(out,"time_uncertainty: %1.1f\n", time_uncertainty);
            fprintf(out,"pos_uncertainty: %3d\n", pos_uncertainty);
        }

        void ConvertToTleData(FILE* out = stdout)
        {
            char tempString[70];
            int checksum = 0;
            //TLE Line 0, title line
            fprintf(out, "ISON-%d\n", sequence);
            int launch_year = date_1st_measurement % 100; //last two digits
            int epoch_year = reference_date % 100; // last two digits
            
            tm date = {};
            date.tm_year = (reference_date % 10000) -1900; //year relative to 1900
            date.tm_mon = (reference_date / 10000) % 100 -1;
            date.tm_mday = (reference_date / 1000000);
            mktime(&date);
            int dayNumber = date.tm_yday;
            int secs = (reference_tod % 100);
            int mins = (reference_tod / 100) % 100;
            int hrs  = ((reference_tod) / 10000) % 100;
            float tod = (float)(((((hrs * 60) + mins) * 60) + secs))/(float)(24*60*60);
            float doy = (float)dayNumber + tod;

            //TLE Line 1
            sprintf(tempString, "1 %5.5uU %2.2u001A   %2.2u%012.8f 0.00000000  00000-0  00000-0 0    0",
                so_num, 
                launch_year,
                epoch_year,
                doy);
            checksum = calculateChecksum(tempString);
            fprintf(out,"%s%1u\n", tempString, checksum);

            float mean_anomaly = latitude_at_reference - perigee;
            if(mean_anomaly < 0) mean_anomaly += 360.0;
            float mean_motion = orbital_properties(major_semi_axis, eccentricity);
            eccentricity = eccentricity * 10000000;
            //TLE Line 2
            sprintf(tempString, "2 %5.5u %8.4f %8.4f %07.0f %8.4f %8.4f %11.8f    0",
                so_num, 
                inclination,
                right_ascension,
                eccentricity,
                perigee,
                mean_anomaly,
                mean_motion);
            checksum = calculateChecksum(tempString);
            fprintf(out,"%s%1u\n", tempString, checksum);

        }
}; 

int main(int argc, char*argv[])
{
    FILE *inputFILE;
    if (argc < 1) return(-1);
    std::string inputFileName = argv[1];
    inputFILE = fileOpen(argv[1], "r");

    // if(inputFILE == 0) return(-2);
    // char name_card[TLE_LINE_LENGTH];
    // char second_card[TLE_LINE_LENGTH];
    // char third_card[TLE_LINE_LENGTH];
    //  fgets(name_card, sizeof(name_card), inputFILE);  // get first line of TLE
    // fgets(second_card, sizeof(second_card), inputFILE);  // get first line of TLE, outside of the while
    // fgets(third_card, sizeof(third_card), inputFILE);  // outside of the while loop
    // Tle tle = Tle(name_card, second_card, third_card);
    // tle.printTLEFormat();

    char line[250];
    while(fgets(line, sizeof(line), inputFILE))
    {
        std::cout << line << std::endl;
        Vimpel isonObj = Vimpel(line);
        isonObj.print();
        printf("TLE Equivalent:\n");
        isonObj.ConvertToTleData();
        printf("\n\n\n");
    }
}