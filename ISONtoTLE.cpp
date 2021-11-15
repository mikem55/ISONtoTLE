#include "utility.h"

#include <ctype.h>
#include <string.h> // strtok, strcpy stuff not even used yet
#include <iostream>
#include <fstream>
#include <stdlib.h> // atoi, atof not used yet
#include <math.h>  // math functions
#include <time.h>

#define MU  398600.4418  // gravitational parameter
#define MIN_PER_DAY  24 * 60  // minutes per day
#define SEC_PER_DAY  (MIN_PER_DAY * 60)   //seconds per day
#define TLE_LINE_LENGTH 80

using namespace std;

int calculateTLEChecksum(char *line) 
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

struct TLE_Lines_struct_def { 
    std::string line1, line2, line3;
};
typedef struct TLE_Lines_struct_def TLE_Lines_Type;

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


class ISON
{
    public:
        std::string ISON_line;
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
        ISON(char* line)
        {
            ISON_line = std::string(line);
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

        TLE_Lines_Type ConvertToTleLines(FILE* out = stdout)
        {
            TLE_Lines_Type TLE_Equiv;
            char tempString[70], tempString2[80];
            int checksum = 0;
            
            //TLE Line 0, title line
            sprintf(tempString, "ISON-%d", sequence);
            TLE_Equiv.line1 = std::string(tempString);
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
            checksum = calculateTLEChecksum(tempString);
            sprintf(tempString2,"%s%1u", tempString, checksum);
            TLE_Equiv.line2 = std::string(tempString2);

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
            checksum = calculateTLEChecksum(tempString);
            sprintf(tempString2,"%s%1u", tempString, checksum);
            TLE_Equiv.line3 = std::string(tempString2);
            return TLE_Equiv;
        }
}; 

int main(int argc, char*argv[])
{
    FILE *inputFILE;
    FILE *outputFILE;

    if (argc < 2) 
    {
        
        cout << "Error - filename[s] missing." << endl << "Usage: " << endl;
        cout << argv[0] << " inputfilename outputfilename" << endl;
        cout << "\tinputfilename = file containing ISON entries" << endl;
        cout << "\toutputfilename = file to store TLE conversions (terminal if unspecified)" << endl;

        return(-1);
    }
    char *inputFileName = argv[1];
    inputFILE = fileOpen(argv[1], "r");
        cout << "in:  " << inputFileName << endl;
    if(inputFILE == 0) 
    {
        cout << "Error - Can't open input file: " << inputFileName << endl;
        return(-2);
    }

    if (argc > 2) //2nd parameter was given
    {
        char *outputFileName = argv[2];
        cout << "out: " << outputFileName << endl;
        outputFILE = fileOpen(argv[2], "w");

        if (outputFILE == 0)
        {
            cout << "Error - Can't open output file: " << outputFileName << endl;
            return(-2);
        }
    }
    else
    {
        outputFILE = stdout;
    }

    char line[250];
    TLE_Lines_Type TLE_Equiv;
    while(fgets(line, sizeof(line), inputFILE))
    {
        line[strcspn(line, "\r\n")] = 0; //strip CR & LF from end of line
        if (strlen(line) > 0) // check for empty lines
        {   
            ISON isonObj = ISON(line);
            TLE_Equiv = isonObj.ConvertToTleLines();

    //        isonObj.print();
    //        printf("TLE Equivalent:\n");
            fprintf(outputFILE, "%s\n", TLE_Equiv.line1.c_str());
            fprintf(outputFILE, "%s\n", TLE_Equiv.line2.c_str());
            fprintf(outputFILE, "%s\n", TLE_Equiv.line3.c_str());
            fprintf(outputFILE, "\n");
        }
    }
}