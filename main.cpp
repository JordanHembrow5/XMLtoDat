/*
    PLAN:
        1) Grab Metadata
            - Find the first < and the first > of each line (in metadata section)
            - save the string between those two (metadata title[n])
            - find the first > and the last <
            - save the string between those two (metadata[n])
        2) Add metadata to the top of the .dat file?
            - for n=0->n, print '# metadata title[n]: metadata[n]'
        3) Get x-z data
            - like grabbing metadata or using atof or combo?
 */


#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>


#define DEFAULT_FILENAME (char*)"XML_files/test.xml"
#define MAX_LINE_LEN 100
#define METADATA_COUNT 7


enum cartesianCoords{
    X, Y, Z, N_COORDS
};

typedef struct profileMetadata {
    char date[MAX_LINE_LEN] = "\n";
    char time[MAX_LINE_LEN] = "\n";
    char x_unit[MAX_LINE_LEN] = "\n";
    char z_unit[MAX_LINE_LEN] = "\n";
    int data_points = 0;
    double gain = 0;
    double offset = 0;
}profileMetadata;

typedef struct profileData {
    double x;
    double z;
}profileData;


void removeClosingTag(char* str);
void obtainMetadata(FILE* input_file, profileMetadata *meta);
void obtainData(FILE* input_file, std::vector<profileData> &data);
void outputDat(char* output_file, profileMetadata *meta, const std::vector<profileData> &data);
void XMLtoDat(const char* filenameXML);


int main(int argc, char **argv) {

    /* Obtain a custom filename at the command line. Default used for testing purposes. */
    char* filenameXML = DEFAULT_FILENAME;
    if(argc > 1) {
        filenameXML = argv[1];
    }

    XMLtoDat(filenameXML);
    return 0;
}

/* This function replaces the start of of the closing XML tag with a terminating char, so that the rest is never read */
void removeClosingTag(char* str) {
    char* position = std::strchr(str, '<');
    *position = '\0';
}

/* Grab the metadata from the XML file */
void obtainMetadata(FILE* input_file, profileMetadata* meta) {

    int metadata_count = 0;
    char input_line[MAX_LINE_LEN];
    char throwaway_str[MAX_LINE_LEN];
    while(std::fgets(input_line, MAX_LINE_LEN, input_file) and metadata_count < METADATA_COUNT) {

        if(std::strncmp(input_line, "        <TestDate>", strlen("    <TestDate>")) == 0) {
            std::sscanf(input_line, "        <TestDate>%s</TestDate>", meta->date);
            removeClosingTag(meta->date);
            metadata_count++;
        }
        else if(std::strncmp(input_line, "        <TestTime>", strlen("        <TestTime>")) == 0) {
            std::sscanf(input_line, "\t\t<TestTime>%s</TestTime>", meta->time);
            removeClosingTag(meta->time);
            metadata_count++;
        }
        else if(std::strncmp(input_line, "        <XUnits>", strlen("        <XUnits>")) == 0) {
            std::sscanf(input_line, "\t\t<XUnits>%s</XUnits>", meta->x_unit);
            removeClosingTag(meta->x_unit);
            metadata_count++;
        }
        else if(std::strncmp(input_line, "        <ZUnits>", strlen("        <ZUnits>")) == 0) {
            std::sscanf(input_line, "\t\t<ZUnits>%s</ZUnits>", meta->z_unit);
            removeClosingTag(meta->z_unit);
            metadata_count++;
        }
        else if(std::strncmp(input_line, "        <NumData>", strlen("        <NumData>")) == 0) {
            std::sscanf(input_line, "        <NumData>%i %s", &meta->data_points, throwaway_str);
            metadata_count++;
        }
        else if(std::strncmp(input_line, "        <DataGain>", strlen("        <DataGain>")) == 0) {
            std::sscanf(input_line, "        <DataGain>%lg %s", &meta->gain, throwaway_str);
            metadata_count++;
        }
        else if(std::strncmp(input_line, "        <DataOffset>", strlen("        <DataOffset>")) == 0) {
            std::sscanf(input_line, "        <DataOffset>%lg %s", &meta->offset, throwaway_str);            metadata_count++;
        }
    }

    if(metadata_count < METADATA_COUNT)
        std::cout << "Error. Not all metadata available. Missing " << (7-metadata_count) << std::endl;
}

/* Grab the main data from the XML file */
void obtainData(FILE* input_file, std::vector<profileData> &data) {

    char input_line[MAX_LINE_LEN];
    char throwaway_str[MAX_LINE_LEN];
    int x_ctr = 0, z_ctr = 0, diff_ctr = 0;
    profileData temp;
    while(std::fgets(input_line, MAX_LINE_LEN, input_file)) {
        if(std::strncmp(input_line, "            <X>", strlen("            <X>")) == 0) {
            std::sscanf(input_line, "            <X>%lg%s", &data[x_ctr].x, throwaway_str);
            x_ctr++;
        }
        else if(std::strncmp(input_line, "            <Z>", strlen("            <Z>")) == 0) {
            std::sscanf(input_line, "            <Z>%lg%s", &data[z_ctr].z, throwaway_str);
            z_ctr++;
        }

        /* Check for unpaired value. The diff_ctr ensures that fgets has got to the next x value before saying it hasn't found a z value */

        if(x_ctr != z_ctr) {
            if(diff_ctr > 1) {
                std::cout << "Error, unpaired value at index " << x_ctr << "/" << z_ctr << std::endl;
            }
            diff_ctr++;
        }
        else {
            diff_ctr = 0;
        }

    }
    if(x_ctr != z_ctr) {
        std::cout << "Error! Unpaired x-z values" << std::endl;
        exit(2);
    }
}

/* Output the obtained data to a .dat formatted file */
void outputDat(char* output_file, profileMetadata *meta, const std::vector<profileData> &data) {
    std::ofstream file;
    file.open(output_file);

    /* Metadata first */
    file << "# Date: " << meta->date << " " << meta->time << std::endl;
    file << "# Units - x: " << meta->x_unit << "\tz: " << meta->z_unit << std::endl;
    file << "# Data Points: " << meta->data_points << std::endl;
    file << "# Gain: " << meta->gain << std::endl;
    file << "# Offset: " << meta->offset << std::endl;

    for(auto ele : data) {
        file << ele.x << "\t" << ele.z << std::endl;
    }
}


/*
 * Converts the XML format from the profiler to a .dat file (tab separated) while preserving the metadata using # to comment out the lines
 */
void XMLtoDat(const char* filenameXML) {

    FILE* input_file = std::fopen(filenameXML, "r");
    if(!input_file) {
        std::cout << "Error, unable to open file: " << filenameXML << std::endl;
        exit(1);
    }

    profileMetadata meta;
    obtainMetadata(input_file, &meta);

    std::vector<profileData> data;
    data.resize((u_long)meta.data_points);
    obtainData(input_file, data);

    outputDat((char*)"testing_out.dat", &meta, data);
}
