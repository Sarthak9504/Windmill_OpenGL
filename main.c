#define _CRT_SECURE_NO_WARNINGS // to supress CRT lib warnings like for scanf into buffer.
#define _WINSOCK_DEPRECATED_NO_WARNINGS // for supressing depreciated resources warning.
#define _WINSOCKAPI_ // to avoid clash of windows.h and winsock.h


#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<GL/glut.h>
#include<math.h>
#include <Windows.h>
#include <winhttp.h>
#include <Commdlg.h>
#include <tchar.h>
#include <time.h>

// pragma is used to give compiler specific instructions for changing the way the code is compiled/processed.
// Here comment(lib, ...) tells compiler to link given file while the linking process.
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winhttp.lib")

#define pi 3.14 // mathematics: pi
#define dt 0.005 // delta time per frame
#define HEIGHT 600
#define WIDTH 600



// Define a structure to store the TGA file information
typedef struct {
    char idLength;
    char colourMapType;
    char imageType;
    short colourMapStart;
    short colourMapNumEntries;
    char bitsPerEntry;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char* pixelData;
} TGAFILE;

// we use this to store node in pair format.
// At a time only two nodes in this linked list
struct ParsedInfoNode {
    int timestamp;
    float wind_mps;
    float density;
    char time_Node[100];
    struct ParsedInfoNode* next;
}; // Two nodes are needed per hour calculations of rpm and power
struct ParsedInfoNode* firstParsedInfoNode = NULL;

// we use this to store all the rpms and power generation per hour
struct windmill {
    char nodeTitle[100];
    float rpm;
    double power_gen;
    struct windmill* next;
};
struct windmill* wHead = NULL;

struct windmill_parameters {
    float len;
    float kw;
    float km;
    float ke;
    float ket;
    float kt;
    float h;
    char x[10];
    char y[10];
    float cp;
    float tsr;
};
struct windmill_parameters* param = NULL;


// display objects
GLfloat** bladeVertices = NULL;     // vertices
GLint** bladeFaces = NULL;          // faces
GLfloat** bladeNormals = NULL;      // all normals
GLint** bladeFaceNormals = NULL;    // normal indices
int* bladeArrSizes = NULL;      // {vertices_size, faces_size}

GLfloat** bodyVertices = NULL;  // vertices
GLint** bodyFaces = NULL;       // faces
GLfloat** bodyNormals = NULL;      // all normals
GLint** bodyFaceNormals = NULL;    // normal indices
int* bodyArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** terrainVertices = NULL;  // vertices
GLint** terrainFaces = NULL;       // faces
GLfloat** terrainNormals = NULL;      // all normals
GLint** terrainFaceNormals = NULL;    // normal indices
int* terrainArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** skyboxVertices = NULL;  // vertices
GLint** skyboxFaces = NULL;       // faces
GLfloat** skyboxNormals = NULL;      // all normals
GLint** skyboxFaceNormals = NULL;    // normal indices
int* skyboxArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** cloudVertices = NULL;  // vertices
GLint** cloudFaces = NULL;       // faces
GLfloat** cloudNormals = NULL;      // all normals
GLint** cloudFaceNormals = NULL;    // normal indices
int* cloudArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** baseVertices = NULL;  // vertices
GLint** baseFaces = NULL;       // faces
GLfloat** baseNormals = NULL;      // all normals
GLint** baseFaceNormals = NULL;    // normal indices
int* baseArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** shaftVertices = NULL;  // vertices
GLint** shaftFaces = NULL;       // faces
GLfloat** shaftNormals = NULL;      // all normals
GLint** shaftFaceNormals = NULL;    // normal indices
int* shaftArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** shaftHolderVertices = NULL;  // vertices
GLint** shaftHolderFaces = NULL;       // faces
GLfloat** shaftHolderNormals = NULL;      // all normals
GLint** shaftHolderFaceNormals = NULL;    // normal indices
int* shaftHolderArrSizes = NULL;       // {vertices_size, faces_size}

GLfloat** shaftBrakeVertices = NULL;  // vertices
GLint** shaftBrakeFaces = NULL;       // faces
GLfloat** shaftBrakeNormals = NULL;      // all normals
GLint** shaftBrakeFaceNormals = NULL;    // normal indices
int* shaftBrakeArrSizes = NULL;       // {vertices_size, faces_size}

// properties of display objects
float angularSpeed = -pi / 30, angle = 0; // angularSpeed in radians/second, angle in radians
float WindmillBodyScale = 3; // scale w.r.t parsed object
float WindmillBladeScale = 3; // scale w.r.t parsed object
float cloudSpeed = 0.03;
float cloudPos = 0;
float cameraHorizMovement = 0;
float cameraVertMovement = 2;
int screenHeight = 0, screenWidth = 0;

int takingInput = 1; // = 1 when we are taking input from user
char input[3][15] = { '\0' };
char inputTexts[3][15] = { "X coor\0", "Y coor", "BladeLen" };
int inputFilled = 0;
int inputNumber = 0;
int fetched_and_called = 0;
char csv_keys[20][100] = { "" }; // Keys inside csv. CSV = (key, value) pairs
char csv_values[20][100] = { "" }; // Values inside csv. CSV = (key, value) pairs
char last_updated_at[20];
char name_of_place[10];
char region[10];
char country[10];
float instantaneous_rpm;
double instantaneous_power_gen;

struct windmill* displayTemp = NULL;
int currentFrame = 0;
char current_windmill_details_for_display[200] = "";
char current_place[200] = "";
char current_time[100] = "";
char current_windmill_details_for_display_rpm[200] = "";
char current_windmill_details_for_display_time_interval[200] = "";
char current_windmill_details_for_display_power[200] = "";
char instantaneous_power_gen_string[200] = "";

int opened_up_view = 0;

GLfloat colorDarkGray[] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat colorLightGray[] = { 0.3, 0.3, 0.3, 1.0 };
GLfloat colorDarkGray2[] = { 0.4, 0.4, 0.4, 1.0 };

GLfloat light_position1[] = { 50.f, 10.f, 50.f, 1.0f };
GLfloat light_position2[] = { 50.f, 10.f, 0.f, 1.0f };

int rpm = 20;


TGAFILE tgaFile;

// functions
void calculate_power_and_rpm();
double instantaneous_power(double v_1, double d_1);
double calculateExpression(double t_1, double t_2, double v_1, double v_2, double d_1, double d_2);
int setWindmillConstantParameters();
void importMeshes();
int* makeSpaceForModel(const char*, GLfloat***, GLint***, GLfloat***, GLint***);
void parseOBJFile(const char*, GLfloat***, GLint***, GLfloat***, GLint***);
void init();
void initLight();
void reshape(int, int);
void display();
void frameUpdate(int);
void drawBlade();
void drawBody();
void drawInternals();
void drawTerrain();
void drawSkybox();
void drawCloud();
void drawBase();
void rotationMatrixUpdate();
char* call_api();
void insertInParsedInfoPair(struct ParsedInfoNode* ptr);
void insert_w(struct windmill* ptr);
char* search(char* json, char* key);
float windmill_rpm(float tsr, float blade_len, float v1, float v2);
void update_windmill();
void JSONparsing(char* start, int count);
void keyPressed(unsigned char key, int x, int y);
int chooseCSV(char* FileName);
void read_csv(char* file_name);
TGAFILE loadImage(const char* pathToImage);
void getMouseCLicks(int button, int state, int x, int y);
void format_time(char* output);
void format_time_for_csv(char* output);
void write_csv();


void renderBitmapString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y);
    //glScalef(2.0, 2.0,1.0f);

    for (const char* c = string; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }
}



int main(int argc, char** argv) {
    //calculate_power_and_rpm();
    tgaFile = loadImage("D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\StartScreen.tga");
    importMeshes();
    if (bladeArrSizes == NULL) return -1;
    if (bodyArrSizes == NULL) return -1;
    if (terrainArrSizes == NULL) return -1;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

    glutInitWindowPosition(300, 0);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Digital Twin: Windmill");
    glEnable(GL_DEPTH_TEST);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, frameUpdate, 0);
    printf("4");
    glutDisplayFunc(display);
    printf("5");
    init();
    printf("2");
    glutMainLoop();
    return 0;
}

// Load a TGA file from a given path and store it in the structure
TGAFILE loadImage(const char* pathToImage) {
    FILE* image = fopen(pathToImage, "rb");
    if (!image) {
        printf("Error: File not found.\n");
        exit(1);
    }
    TGAFILE tgaFile;
    fread(&tgaFile.idLength, sizeof(tgaFile.idLength), 1, image);
    fread(&tgaFile.colourMapType, sizeof(tgaFile.colourMapType), 1, image);
    fread(&tgaFile.imageType, sizeof(tgaFile.imageType), 1, image);
    fread((char*)(&tgaFile.colourMapStart), sizeof(tgaFile.colourMapStart), 1, image);
    fread((char*)(&tgaFile.colourMapNumEntries), sizeof(tgaFile.colourMapNumEntries), 1, image);
    fread(&tgaFile.bitsPerEntry, sizeof(tgaFile.bitsPerEntry), 1, image);
    fread((char*)(&tgaFile.xOrigin), sizeof(tgaFile.xOrigin), 1, image);
    fread((char*)(&tgaFile.yOrigin), sizeof(tgaFile.yOrigin), 1, image);
    fread((char*)(&tgaFile.width), sizeof(tgaFile.width), 1, image);
    fread((char*)(&tgaFile.height), sizeof(tgaFile.height), 1, image);
    fread(&tgaFile.bitsPerPixel, sizeof(tgaFile.bitsPerPixel), 1, image);

    int imageDataSize = tgaFile.width * tgaFile.height * (tgaFile.bitsPerPixel / 8);
    tgaFile.pixelData = (char*)malloc(imageDataSize);
    fread(tgaFile.pixelData, imageDataSize, 1, image);

    // Swap BGR order to RGB order
    for (int i = 0; i < imageDataSize; i += (tgaFile.bitsPerPixel / 8)) {
        char temp = tgaFile.pixelData[i];
        tgaFile.pixelData[i] = tgaFile.pixelData[i + 2];
        tgaFile.pixelData[i + 2] = temp;
    }

    fclose(image);
    return tgaFile;
}


void read_csv(char* file_name) {
    int csv_iterator = 0;
    int key_number = 0; // nth key
    int specific_key_iterator = 0; // nth char in key
    int value_number = 0; // nth value
    int specific_value_iterator = 0; // nth char in value
    int is_key = 1;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 100; j++) {
            csv_keys[i][j] = '\0';
            csv_values[i][j] = '\0';
        }
    }

    //Read file
    FILE* csv_ptr;
    char csv_char;
    csv_ptr = fopen(file_name, "r");
    if (NULL == csv_ptr) {
        printf("CSV file can't be opened \n");
        return;
    }

    csv_char = fgetc(csv_ptr);
    while (csv_char != EOF) {
        printf("%c", csv_char);
        if (csv_char == ',') {
            is_key = 0;
            specific_key_iterator = 0;
            key_number++;
        }
        else if (csv_char == '\n') {
            is_key = 1;
            specific_value_iterator = 0;
            value_number++;
        }
        else if (is_key == 1) {
            csv_keys[key_number][specific_key_iterator] = csv_char;
            specific_key_iterator++;
        }
        else {
            csv_values[value_number][specific_value_iterator] = csv_char;
            specific_value_iterator++;
        }
        csv_char = fgetc(csv_ptr);
    }
    for (int i = 0; i < 20; i++) {
        printf("\n   %s : %s", csv_keys[i], csv_values[i]);
    }

    printf("\n--------------------------------------------------");

    struct windmill_parameters* ptr = malloc(sizeof(struct windmill_parameters));
    param = ptr;
    strncpy(param->x, csv_values[0], 10);
    printf("X - %s", param->x);
    param->x[9] = '\0';
    strncpy(param->y, csv_values[1], 10);
    param->y[9] = '\0';
    param->len = strtod(csv_values[2], NULL);
    param->h = strtod(csv_values[3], NULL);
    param->kw = strtod(csv_values[4], NULL);
    param->km = strtod(csv_values[5], NULL);
    param->ke = strtod(csv_values[6], NULL);
    param->ket = strtod(csv_values[7], NULL);
    param->kt = strtod(csv_values[8], NULL);
    param->tsr = strtod(csv_values[9], NULL);
    param->cp = strtod(csv_values[10], NULL);
    printf("\nh = %f, %p", param->h, param);
    printf("\n%f", param->ke);
}

// pick a csv file
int chooseCSV(char* FileName) {
    OPENFILENAME  ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    wchar_t szFile[25 * MAX_PATH];
    wchar_t szPath[MAX_PATH];
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    //ofn.hInstance = NULL;
    ofn.lpstrFilter = L"CSV Files\0*.csv\0\0";
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    //ofn.lpstrTitle = L"Please Select A File To Open";
    //ofn.nMaxFile = 1000;
    ofn.Flags = OFN_NONETWORKBUTTON |
        OFN_FILEMUSTEXIST |
        OFN_HIDEREADONLY;
    if (!GetOpenFileName(&ofn))
        return(0);
    //printf("The selected file path is - %s\n", ofn.lpstrFile);
    /*for (int i = 0; i < 100; i++) {
        printf("\n%d %c", ofn.lpstrFile[i], ofn.lpstrFile[i]);
    }*/
    int i = 0;
    while (ofn.lpstrFile[i] != 0) {
        FileName[i] = ofn.lpstrFile[i];
        i++;
    }
    FileName[i] = '\0';
    return 1;
}

void calculate_power_and_rpm() {
    char* buffer = call_api(); // Get Data from Website to Buffer through API call //printf("%s", buffer);
    //if (setWindmillConstantParameters() == 0) return; // set constant parameters of windmill & return if unsucessful
    int initial_count = 0;

    //param = malloc(sizeof(struct windmill_parameters));


    JSONparsing(buffer, initial_count);

    // Now you can work with the content stored in 'buffer'
    // Don't forget to free the allocated memory when done
    free(buffer);

    //long p = power_gen_perhr(1.166176, 1.68501, 6.805555, 7.00000, 1691929800, 1691933400, param->len, CP, param->kw, param->km, param->ke, param->ket, param->kt);
    //printf("%f",p );
}

int setWindmillConstantParameters() {
    //param = malloc(sizeof(struct windmill_parameters));
    if (param == NULL) return 0;
    param->ke = 1.5 / 100;
    param->ket = 5.0 / 100;
    param->km = 0.2 / 100;
    param->kt = 3.0 / 100;
    param->kw = 0.2 / 100;
    sscanf(csv_values[2], "%f", &param->len);
    printf("\n\n-------------------\nLen : %f\n\n--------------------------------------------------\n", param->len);
    return 1;
}

void insertInParsedInfoPair(struct ParsedInfoNode* ptr) {
    if (firstParsedInfoNode == NULL) {
        // first node
        firstParsedInfoNode = ptr;
        firstParsedInfoNode->next = NULL;
    }
    else if (firstParsedInfoNode->next == NULL) {
        // second node
        firstParsedInfoNode->next = ptr;
        firstParsedInfoNode->next->next = NULL;
    }
    else {
        // Remove the prev node (the head) and free its memory
        // we do this since we only want two latest info nodes in the current info pair
        struct ParsedInfoNode* prevParsedInfoNode = firstParsedInfoNode;
        firstParsedInfoNode = firstParsedInfoNode->next;
        free(prevParsedInfoNode);
        firstParsedInfoNode->next = ptr;
        ptr->next = NULL;
    }
}


void insert_w(struct windmill* ptr) {
    if (wHead == NULL) {
        wHead = ptr;
        ptr->next = NULL;
    }
    else {
        struct windmill* temp = wHead;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = ptr;
        ptr->next = NULL;
    }
}


char* search(char* json, char* key) {
    if (json == NULL) {
        return NULL;
    }
    if (key == NULL) {
        return NULL;
    }
    char* start = strstr(json, key);
    if (!start) {
        return NULL;
    }

    start = start + strlen(key) + 2;
    return start;
}



double calculateExpression(double t_1, double t_2, double v_1, double v_2, double d_1, double d_2) {
    double A = pi * pow(param->len, 2);
    double k = param->cp * (1 - param->km) * (1 - param->ke) * (1 - param->ket) * (1 - param->kt) * (1 - param->kw);
    double result = (A * k * (t_2 - t_1) * (
        ((4 * d_2 + d_1) * pow(v_2, 3) + (3 * d_2 + 2 * d_1) * v_1 * pow(v_2, 2) +
            (2 * d_2 + 3 * d_1) * pow(v_1, 2) * v_2 + (d_2 + 4 * d_1) * pow(v_1, 3))
        ) / (40 * t_2));
    int no_of_windmills = atoi(csv_values[11]);
    return (result / 1000) * no_of_windmills;
}



float windmill_rpm(float tsr, float blade_len, float v1, float v2) {
    float wind_vel = (v1 + v2) / 2;
    float rpm = (30 * wind_vel * tsr) / (pi * blade_len);
    return rpm;
}



void update_windmill() {
    struct ParsedInfoNode* temp = firstParsedInfoNode;
    struct windmill* ptr123 = malloc(sizeof(struct windmill));
    if (ptr123 == NULL) {
        perror("Memory allocation unsuccessful for windmill update function");
        return;
    }
    ptr123->power_gen = calculateExpression(0, temp->next->timestamp - temp->timestamp, temp->wind_mps, temp->next->wind_mps, temp->density, temp->next->density, 38.4845, 0.26);
    ptr123->rpm = windmill_rpm(param->tsr, param->len, temp->wind_mps, temp->next->wind_mps);
    //printf("\n%s", temp->time_Node);
    strcpy(ptr123->nodeTitle, temp->time_Node);
    strcat(ptr123->nodeTitle, " to ");
    strcat(ptr123->nodeTitle, temp->next->time_Node);
    ptr123->next = NULL;
    // DEBUGGING STATEMENTS
    printf("\n\nTime interval: %s", ptr123->nodeTitle);
    printf("\n\nPower: %.5f", ptr123->power_gen);
    printf("\nrpm : %f", ptr123->rpm);
    printf("\n---------------------------------------------------------------------------------\n");

    insert_w(ptr123);
}



double instantaneous_power(double v_1, double d_1)
{
    double A = pi * pow(param->len, 2);
    double k = param->cp * (1 - param->km) * (1 - param->ke) * (1 - param->ket) * (1 - param->kt) * (1 - param->kw);
    double result = 1.0 / 2 * d_1 * pow(v_1, 3) * A;
    return result / 1000;
}




void JSONparsing(char* start, int count) {
    count++;
    if (start == NULL) return; // failed to get data from website.

    int time_epoch;
    float temp_c;
    float pressure_mb;
    float wind_kph;

    char* ptr;
    int i = 0;
    char toConvert[12];
    char time_Buffer[20];

    if (count == 1)
    {

        ptr = search(start, "name");
        if (ptr == NULL) return; // failed to find
        while (ptr != NULL && *ptr != ',' && *ptr != '\0') {
            if (*ptr != '"') {
                name_of_place[i] = *ptr;
                i += 1; // next index
            }
            ptr += 1; // next char
        }
        i = 0;
        ptr = search(ptr, "region");
        if (ptr == NULL) return; // failed to find
        while (ptr != NULL && *ptr != ',' && *ptr != '\0') {
            if (*ptr != '"') {
                region[i] = *ptr;
                i += 1; // next index
            }
            ptr += 1; // next char
        }
        i = 0;
        ptr = search(ptr, "country");
        if (ptr == NULL) return; // failed to find
        while (ptr != NULL && *ptr != ',' && *ptr != '\0') {
            if (*ptr != '"') {
                country[i] = *ptr;
                i += 1; // next index
            }
            ptr += 1; // next char


            ptr = search(ptr, "last_updated_epoch");

            ptr = search(ptr, "last_updated");
            if (ptr == NULL) return; // failed to find
            while (ptr != NULL && *ptr != ',' && *ptr != '\0') {
                if (*ptr != '"') {
                    last_updated_at[i] = *ptr;
                    i += 1; // next index
                }
                ptr += 1; // next char
            }
            i = 0;







        }
        printf("\n\n********************************************************************");
        printf("\n\nLocation : %s, %s, %s", name_of_place, region, country);
        printf("\n\nLast Updated time: %s", last_updated_at);
        printf("\n\n********************************************************************");

    }



    i = 0;


    ptr = search(start, "time_epoch");
    if (ptr == NULL) return; // failed to find
    while (ptr != NULL && *ptr != ',' && *ptr != '\0') {
        toConvert[i] = *ptr;
        i += 1; // next index
        ptr += 1; // next char
    }
    toConvert[i] = '\0'; // end the string to avoid over
    time_epoch = atoi(toConvert); // save info in integer format



    i = 0;
    ptr = search(ptr, "time");
    if (ptr == NULL) return; // failed to find
    while (ptr != NULL && *ptr != ',' && *ptr != '\0' && *ptr != '}') {
        if (*ptr == '"') {
            ptr++;
            continue;
        }
        time_Buffer[i] = *ptr;
        i += 1; // next index
        ptr += 1; // next char
    }
    time_Buffer[i] = '\0'; // end the string to avoid over



    i = 0;
    ptr = search(ptr, "temp_c"); // move pointer to next info
    if (ptr == NULL) return; // failed to find
    while (*ptr != ',' && *ptr != '\0') {
        toConvert[i] = *ptr;
        i = i + 1;
        ptr = ptr + 1;
    }
    toConvert[i] = '\0'; // end the string to avoid over
    temp_c = atof(toConvert); // save info in integer format



    i = 0;
    ptr = search(ptr, "wind_kph"); // move pointer to next info
    if (ptr == NULL) return; // failed to find
    while (*ptr != ',' && *ptr != '\0') {
        toConvert[i] = *ptr;
        i = i + 1;
        ptr = ptr + 1;
    }
    toConvert[i] = '\0'; // end the string to avoid over
    wind_kph = atof(toConvert); // save info in integer format



    i = 0;
    ptr = search(ptr, "pressure_mb"); // move pointer to next info
    if (ptr == NULL) return; // failed to find
    while (*ptr != ',' && *ptr != '\0') {
        toConvert[i] = *ptr;
        i = i + 1;
        ptr = ptr + 1;
    }
    toConvert[i] = '\0'; // end the string to avoid over
    pressure_mb = atof(toConvert); // save info in integer format


    float temp_k = temp_c + 273; // temperature from Celsius to Kelvin
    float wind_mps = wind_kph / 3.6; // wind speed from kilometer per hour to miles per hour
    float density = pressure_mb * 0.348 / temp_k; // calculate density from pressure and temperature in kelvin


    // DEBUGGING FINAL FORMAT VALUES
    //printf("\nTime: %s",time_Buffer);
    //printf("\n\n\ntime-stamp: %d", time_epoch);
    //printf("\ntemperature in deg k: %f", temp_k);
    //printf("\nwind speed mps: %f", wind_mps);
    //printf("\nDensity: %f", density);


    // organize collected data to structure
    struct ParsedInfoNode* temp = malloc(sizeof(struct ParsedInfoNode));
    if (temp == NULL) perror("The Memory Allocation Was Unsucessful for Node (to store the Parsed data)");
    temp->timestamp = time_epoch;
    strcpy(temp->time_Node, time_Buffer);
    temp->density = density;
    temp->wind_mps = wind_mps;
    temp->next = NULL;
    insertInParsedInfoPair(temp); // we put the collected data into pair list of info


    if (count == 1) { // if it is first node, then add one more since we need atleast two to start updating it to windmill linkedlist.
        if (ptr == NULL) {
            printf("\n\nEnd of file");
            return;
        }
        else {
            instantaneous_power_gen = instantaneous_power(wind_mps, density);
            sprintf(instantaneous_power_gen_string, "    Current Power Generated : %.5f kWh", instantaneous_power_gen);
            printf("\n====================================== Instantaneous Power : %f", instantaneous_power);
            instantaneous_rpm = windmill_rpm(param->tsr, param->len, wind_mps, wind_mps);


            JSONparsing(ptr, count); // now the ptr points ahead of the visited data
        }
    }
    else { // if not the first node, then can update everytime.
        update_windmill();
        if (ptr == NULL) {
            printf("\n\nEnd of file");
            return;
        }
        else {
            JSONparsing(ptr, count); // now the ptr points ahead of the visited data
        }
    }
}

void importMeshes() {
    // put model paths
    char* windmillBodyFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\Windmill.obj";
    char* windmillBladeFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\StraightBladeCurvedDetailed.obj";
    char* terrainFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\Terrain3.obj";
    //char* skyboxFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\Sky1.obj";
    char* cloudFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\Cloud.obj";
    char* baseFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\Base1.obj";
    char* shaftFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\Shaft.obj";
    char* shaftHolderFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\ShaftHolder.obj";
    char* shaftBrakeFile = "D:\\users\\madhur\\Education\\Engineering\\VIT Pune\\SY\\Sem1\\CGAVR\\Course Project\\ShaftBrake.obj";
    // import models
    bladeArrSizes = makeSpaceForModel(windmillBladeFile, &bladeVertices, &bladeFaces, &bladeNormals, &bladeFaceNormals); // make dynamic memory allocation
    parseOBJFile(windmillBladeFile, &bladeVertices, &bladeFaces, &bladeNormals, &bladeFaceNormals); // parse in allocated memory
    bodyArrSizes = makeSpaceForModel(windmillBodyFile, &bodyVertices, &bodyFaces, &bodyNormals, &bodyFaceNormals); // make dynamic memory allocation
    parseOBJFile(windmillBodyFile, &bodyVertices, &bodyFaces, &bodyNormals, &bodyFaceNormals); // parse in allocated memory
    terrainArrSizes = makeSpaceForModel(terrainFile, &terrainVertices, &terrainFaces, &terrainNormals, &terrainFaceNormals);
    parseOBJFile(terrainFile, &terrainVertices, &terrainFaces, &terrainNormals, &terrainFaceNormals);
    //skyboxArrSizes = makeSpaceForModel(skyboxFile, &skyboxVertices, &skyboxFaces, &skyboxNormals, &skyboxFaceNormals);
    //parseOBJFile(skyboxFile, &skyboxVertices, &skyboxFaces, &skyboxNormals, &skyboxFaceNormals);
    cloudArrSizes = makeSpaceForModel(cloudFile, &cloudVertices, &cloudFaces, &cloudNormals, &cloudFaceNormals);
    parseOBJFile(cloudFile, &cloudVertices, &cloudFaces, &cloudNormals, &cloudFaceNormals);
    baseArrSizes = makeSpaceForModel(baseFile, &baseVertices, &baseFaces, &baseNormals, &baseFaceNormals);
    parseOBJFile(baseFile, &baseVertices, &baseFaces, &baseNormals, &baseFaceNormals);
    shaftArrSizes = makeSpaceForModel(shaftFile, &shaftVertices, &shaftFaces, &shaftNormals, &shaftFaceNormals); // make dynamic memory allocation
    parseOBJFile(shaftFile, &shaftVertices, &shaftFaces, &shaftNormals, &shaftFaceNormals); // parse in allocated memory
    shaftHolderArrSizes = makeSpaceForModel(shaftHolderFile, &shaftHolderVertices, &shaftHolderFaces, &shaftHolderNormals, &shaftHolderFaceNormals); // make dynamic memory allocation
    parseOBJFile(shaftHolderFile, &shaftHolderVertices, &shaftHolderFaces, &shaftHolderNormals, &shaftHolderFaceNormals); // parse in allocated memory
    shaftBrakeArrSizes = makeSpaceForModel(shaftBrakeFile, &shaftBrakeVertices, &shaftBrakeFaces, &shaftBrakeNormals, &shaftBrakeFaceNormals); // make dynamic memory allocation
    parseOBJFile(shaftBrakeFile, &shaftBrakeVertices, &shaftBrakeFaces, &shaftBrakeNormals, &shaftBrakeFaceNormals); // parse in allocated memory
}

int* makeSpaceForModel(const char* filename, GLfloat*** vertices, GLint*** faces, GLfloat*** allNormals, GLint*** faceNormals) {
    FILE* file = fopen(filename, "r");
    int vertexIndex = 0;
    int faceIndex = 0;
    int normalIndex = 0;
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }
    else {
        char lineHeader[128];
        while (fscanf(file, "%s", lineHeader) != EOF) {
            if (strcmp(lineHeader, "v") == 0) vertexIndex++;
            else if (strcmp(lineHeader, "f") == 0) faceIndex++;
            else if (strcmp(lineHeader, "vn") == 0) normalIndex++;
        }
        *vertices = (GLfloat**)malloc(vertexIndex * sizeof(GLfloat*));
        for (int i = 0; i < vertexIndex; i++) *((*vertices) + i) = (GLfloat*)malloc(3 * sizeof(GLfloat));
        *faces = (GLint**)malloc(faceIndex * sizeof(GLint*));
        for (int i = 0; i < faceIndex; i++) *((*faces) + i) = (GLint*)malloc(3 * sizeof(GLint));
        *allNormals = (GLfloat**)malloc(normalIndex * sizeof(GLfloat*));
        for (int i = 0; i < normalIndex; i++) *((*allNormals) + i) = (GLfloat*)malloc(3 * sizeof(GLfloat));
        *faceNormals = (GLint**)malloc(faceIndex * sizeof(GLint*));
        for (int i = 0; i < faceIndex; i++) *((*faceNormals) + i) = (GLint*)malloc(3 * sizeof(GLint));
    }
    fclose(file);
    int* sizes = (int*)malloc(sizeof(int) * 3);
    sizes[0] = vertexIndex;
    sizes[1] = faceIndex;
    sizes[2] = normalIndex;
    //printf("faces: %d\n", faceIndex);
    return sizes;
}

void parseOBJFile(const char* filename, GLfloat*** vertices, GLint*** faces, GLfloat*** allNormals, GLint*** faceNormals) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char lineHeader[128];
    int vertexIndex = 0;
    int faceIndex = 0;
    int normalIndex = 0;
    while (fscanf(file, "%s", lineHeader) != EOF) {
        if (strcmp(lineHeader, "v") == 0) {
            fscanf(file, "%f %f %f", &((*vertices)[vertexIndex][0]), &((*vertices)[vertexIndex][1]), &((*vertices)[vertexIndex][2]));
            vertexIndex++;
        }
        else if (strcmp(lineHeader, "f") == 0) {
            char line[50]; // buffer of parsing
            fgets(line, sizeof(line), file);
            char* token = strtok(line, " ");
            int vertexIndices[5] = { 0 };
            int normalIndices[5] = { 0 };
            int idx = 0;
            while (token != NULL) {
                //printf("%d", token[0]-48);
                int garbage = 0;
                sscanf(token, "%d//%d", &vertexIndices[idx], &normalIndices[idx]);
                idx++;
                token = strtok(NULL, " ");
            }
            for (int i = 0; i < 3; i++) {
                (*faces)[faceIndex][i] = vertexIndices[i];
                (*faceNormals)[faceIndex][i] = normalIndices[i];
            }
            faceIndex++;
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            fscanf(file, "%f %f %f", &((*allNormals)[normalIndex][0]), &((*allNormals)[normalIndex][1]), &((*allNormals)[normalIndex][2]));
            normalIndex++;
        }
    }
    fclose(file);
}

void init() {
    glClearColor(0.0, 0.3, 1, 1.0);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);


    GLfloat mat_diffuse[] = { 0.1, 0.1, 0.1, 1.0 };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    //glColor3f(1, 1, 1);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GLUT_MULTISAMPLE);

    glutKeyboardFunc(keyPressed);
    glutMouseFunc(getMouseCLicks);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 15; j++) {
            input[i][j] = '\0';
        }
    }
}

void initLight() {

    glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT0, GL_AMBIENT, colorDarkGray2);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLightGray);

    glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, colorDarkGray2);
}

void reshape(int w, int h) {
    // This function will be called when the size of window is reshaped.
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)w / (float)h, 2, 200);
    glMatrixMode(GL_MODELVIEW);
    screenHeight = w, screenWidth = h;
}

void xyTOxyz(int w, int h) {
    // This function will be called when moving from 2D to 3D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)w / (float)h, 2, 200);
    glMatrixMode(GL_MODELVIEW);
}

void xyzTOxy(int w, int h) {
    // This function will be called when moving from 3D to 2D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1); // Use orthographic projection for 2D
    glMatrixMode(GL_MODELVIEW);
}

void rotationMatrixUpdate() {
    // This function will be called every time frame updates
    angle += angularSpeed;
    cloudPos += cloudSpeed;
}

void drawBlade() {
    glColor3f(1, 1, 1);
    for (int i = 0; i < bladeArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(-1 * bladeNormals[bladeFaceNormals[i][0] - 1][0], -1 * bladeNormals[bladeFaceNormals[i][0] - 1][1], -1 * bladeNormals[bladeFaceNormals[i][0] - 1][2]);
        glVertex3f(bladeVertices[bladeFaces[i][0] - 1][0] * WindmillBladeScale, bladeVertices[bladeFaces[i][0] - 1][1] * WindmillBladeScale, bladeVertices[bladeFaces[i][0] - 1][2] * WindmillBladeScale);
        glNormal3f(-1 * bladeNormals[bladeFaceNormals[i][1] - 1][0], -1 * bladeNormals[bladeFaceNormals[i][1] - 1][1], -1 * bladeNormals[bladeFaceNormals[i][1] - 1][2]);
        glVertex3f(bladeVertices[bladeFaces[i][1] - 1][0] * WindmillBladeScale, bladeVertices[bladeFaces[i][1] - 1][1] * WindmillBladeScale, bladeVertices[bladeFaces[i][1] - 1][2] * WindmillBladeScale);
        glNormal3f(-1 * bladeNormals[bladeFaceNormals[i][2] - 1][0], -1 * bladeNormals[bladeFaceNormals[i][2] - 1][1], -1 * bladeNormals[bladeFaceNormals[i][2] - 1][2]);
        glVertex3f(bladeVertices[bladeFaces[i][2] - 1][0] * WindmillBladeScale, bladeVertices[bladeFaces[i][2] - 1][1] * WindmillBladeScale, bladeVertices[bladeFaces[i][2] - 1][2] * WindmillBladeScale);
        glEnd();
    }
}

void drawBody() {
    glColor3f(1, 1, 1);
    for (int i = 0; i < bodyArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(bodyNormals[bodyFaceNormals[i][0] - 1][0], bodyNormals[bodyFaceNormals[i][0] - 1][1], bodyNormals[bodyFaceNormals[i][0] - 1][2]);
        glVertex3f(bodyVertices[bodyFaces[i][0] - 1][0] * WindmillBodyScale, bodyVertices[bodyFaces[i][0] - 1][1] * WindmillBodyScale, bodyVertices[bodyFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(bodyNormals[bodyFaceNormals[i][0] - 1][0], bodyNormals[bodyFaceNormals[i][0] - 1][1], bodyNormals[bodyFaceNormals[i][0] - 1][2]);
        glVertex3f(bodyVertices[bodyFaces[i][1] - 1][0] * WindmillBodyScale, bodyVertices[bodyFaces[i][1] - 1][1] * WindmillBodyScale, bodyVertices[bodyFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(bodyNormals[bodyFaceNormals[i][0] - 1][0], bodyNormals[bodyFaceNormals[i][0] - 1][1], bodyNormals[bodyFaceNormals[i][0] - 1][2]);
        glVertex3f(bodyVertices[bodyFaces[i][2] - 1][0] * WindmillBodyScale, bodyVertices[bodyFaces[i][2] - 1][1] * WindmillBodyScale, bodyVertices[bodyFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
}

void drawInternals() {
    glColor3f(1, 0.5, 0);
    for (int i = 0; i < shaftHolderArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][0], shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][1], shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftHolderVertices[shaftHolderFaces[i][0] - 1][0] * WindmillBodyScale, shaftHolderVertices[shaftHolderFaces[i][0] - 1][1] * WindmillBodyScale, shaftHolderVertices[shaftHolderFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][0], shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][1], shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftHolderVertices[shaftHolderFaces[i][1] - 1][0] * WindmillBodyScale, shaftHolderVertices[shaftHolderFaces[i][1] - 1][1] * WindmillBodyScale, shaftHolderVertices[shaftHolderFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][0], shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][1], shaftHolderNormals[shaftHolderFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftHolderVertices[shaftHolderFaces[i][2] - 1][0] * WindmillBodyScale, shaftHolderVertices[shaftHolderFaces[i][2] - 1][1] * WindmillBodyScale, shaftHolderVertices[shaftHolderFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
    glColor3f(0, 0.1, 1);
    for (int i = 0; i < shaftBrakeArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][0], shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][1], shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftBrakeVertices[shaftBrakeFaces[i][0] - 1][0] * WindmillBodyScale, shaftBrakeVertices[shaftBrakeFaces[i][0] - 1][1] * WindmillBodyScale, shaftBrakeVertices[shaftBrakeFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][0], shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][1], shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftBrakeVertices[shaftBrakeFaces[i][1] - 1][0] * WindmillBodyScale, shaftBrakeVertices[shaftBrakeFaces[i][1] - 1][1] * WindmillBodyScale, shaftBrakeVertices[shaftBrakeFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][0], shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][1], shaftBrakeNormals[shaftBrakeFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftBrakeVertices[shaftBrakeFaces[i][2] - 1][0] * WindmillBodyScale, shaftBrakeVertices[shaftBrakeFaces[i][2] - 1][1] * WindmillBodyScale, shaftBrakeVertices[shaftBrakeFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
    //glRotatef(angle, 0, 0, 1);
    //glRotatef(-angle, 0, 0, 1);
}

void drawShaft() {
    glColor3f(1, 1, 0);
    for (int i = 0; i < shaftArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(shaftNormals[shaftFaceNormals[i][0] - 1][0], shaftNormals[shaftFaceNormals[i][0] - 1][1], shaftNormals[shaftFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftVertices[shaftFaces[i][0] - 1][0] * WindmillBodyScale, shaftVertices[shaftFaces[i][0] - 1][1] * WindmillBodyScale, shaftVertices[shaftFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(shaftNormals[shaftFaceNormals[i][0] - 1][0], shaftNormals[shaftFaceNormals[i][0] - 1][1], shaftNormals[shaftFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftVertices[shaftFaces[i][1] - 1][0] * WindmillBodyScale, shaftVertices[shaftFaces[i][1] - 1][1] * WindmillBodyScale, shaftVertices[shaftFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(shaftNormals[shaftFaceNormals[i][0] - 1][0], shaftNormals[shaftFaceNormals[i][0] - 1][1], shaftNormals[shaftFaceNormals[i][0] - 1][2]);
        glVertex3f(shaftVertices[shaftFaces[i][2] - 1][0] * WindmillBodyScale, shaftVertices[shaftFaces[i][2] - 1][1] * WindmillBodyScale, shaftVertices[shaftFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
}

void drawTerrain() {
    glColor3f(0, 1, 0);
    for (int i = 0; i < terrainArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glNormal3f(terrainNormals[terrainFaceNormals[i][0] - 1][0], terrainNormals[terrainFaceNormals[i][0] - 1][1], terrainNormals[terrainFaceNormals[i][0] - 1][2]);
        glVertex3f(terrainVertices[terrainFaces[i][0] - 1][0] * WindmillBodyScale, terrainVertices[terrainFaces[i][0] - 1][1] * WindmillBodyScale, terrainVertices[terrainFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(terrainNormals[terrainFaceNormals[i][1] - 1][0], terrainNormals[terrainFaceNormals[i][1] - 1][1], terrainNormals[terrainFaceNormals[i][1] - 1][2]);
        glVertex3f(terrainVertices[terrainFaces[i][1] - 1][0] * WindmillBodyScale, terrainVertices[terrainFaces[i][1] - 1][1] * WindmillBodyScale, terrainVertices[terrainFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(terrainNormals[terrainFaceNormals[i][2] - 1][0], terrainNormals[terrainFaceNormals[i][2] - 1][1], terrainNormals[terrainFaceNormals[i][2] - 1][2]);
        glVertex3f(terrainVertices[terrainFaces[i][2] - 1][0] * WindmillBodyScale, terrainVertices[terrainFaces[i][2] - 1][1] * WindmillBodyScale, terrainVertices[terrainFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
}

void drawSkybox() {
    glColor3f(0.5, 0.5, 1);
    for (int i = 0; i < skyboxArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);
        glVertex3f(skyboxVertices[skyboxFaces[i][0] - 1][0] * WindmillBodyScale, skyboxVertices[skyboxFaces[i][0] - 1][1] * WindmillBodyScale, skyboxVertices[skyboxFaces[i][0] - 1][2] * WindmillBodyScale);;
        glVertex3f(skyboxVertices[skyboxFaces[i][1] - 1][0] * WindmillBodyScale, skyboxVertices[skyboxFaces[i][1] - 1][1] * WindmillBodyScale, skyboxVertices[skyboxFaces[i][1] - 1][2] * WindmillBodyScale);
        glVertex3f(skyboxVertices[skyboxFaces[i][2] - 1][0] * WindmillBodyScale, skyboxVertices[skyboxFaces[i][2] - 1][1] * WindmillBodyScale, skyboxVertices[skyboxFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
}


void drawCloud() {
    glColor3f(1, 1, 1);
    for (int i = 0; i < cloudArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);

        //normal vector
        GLfloat v1[3], v2[3], n[3];
        v1[1] = cloudVertices[cloudFaces[i][1] - 1][1] - cloudVertices[cloudFaces[i][0] - 1][1];
        v1[2] = cloudVertices[cloudFaces[i][1] - 1][2] - cloudVertices[cloudFaces[i][0] - 1][2];
        v2[0] = cloudVertices[cloudFaces[i][2] - 1][0] - cloudVertices[cloudFaces[i][0] - 1][0];
        v2[1] = cloudVertices[cloudFaces[i][2] - 1][1] - cloudVertices[cloudFaces[i][0] - 1][1];
        v2[2] = cloudVertices[cloudFaces[i][2] - 1][2] - cloudVertices[cloudFaces[i][0] - 1][2];
        n[0] = v1[1] * v2[2] - v1[2] * v2[1];
        n[1] = v1[2] * v2[0] - v1[0] * v2[2];
        n[2] = v1[0] * v2[1] - v1[1] * v2[0];

        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(cloudVertices[cloudFaces[i][0] - 1][0] * WindmillBodyScale, cloudVertices[cloudFaces[i][0] - 1][1] * WindmillBodyScale, cloudVertices[cloudFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(cloudVertices[cloudFaces[i][1] - 1][0] * WindmillBodyScale, cloudVertices[cloudFaces[i][1] - 1][1] * WindmillBodyScale, cloudVertices[cloudFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(cloudVertices[cloudFaces[i][2] - 1][0] * WindmillBodyScale, cloudVertices[cloudFaces[i][2] - 1][1] * WindmillBodyScale, cloudVertices[cloudFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
}


void drawBase() {
    glColor3f(1, 1, 1);
    for (int i = 0; i < baseArrSizes[1]; i++) {
        glBegin(GL_TRIANGLES);

        //normal vector
        GLfloat v1[3], v2[3], n[3];
        v1[1] = baseVertices[baseFaces[i][1] - 1][1] - baseVertices[baseFaces[i][0] - 1][1];
        v1[2] = baseVertices[baseFaces[i][1] - 1][2] - baseVertices[baseFaces[i][0] - 1][2];
        v2[0] = baseVertices[baseFaces[i][2] - 1][0] - baseVertices[baseFaces[i][0] - 1][0];
        v2[1] = baseVertices[baseFaces[i][2] - 1][1] - baseVertices[baseFaces[i][0] - 1][1];
        v2[2] = baseVertices[baseFaces[i][2] - 1][2] - baseVertices[baseFaces[i][0] - 1][2];
        n[0] = v1[1] * v2[2] - v1[2] * v2[1];
        n[1] = v1[2] * v2[0] - v1[0] * v2[2];
        n[2] = v1[0] * v2[1] - v1[1] * v2[0];

        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(baseVertices[baseFaces[i][0] - 1][0] * WindmillBodyScale, baseVertices[baseFaces[i][0] - 1][1] * WindmillBodyScale, baseVertices[baseFaces[i][0] - 1][2] * WindmillBodyScale);
        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(baseVertices[baseFaces[i][1] - 1][0] * WindmillBodyScale, baseVertices[baseFaces[i][1] - 1][1] * WindmillBodyScale, baseVertices[baseFaces[i][1] - 1][2] * WindmillBodyScale);
        glNormal3f(n[0], n[1], n[2]);
        glVertex3f(baseVertices[baseFaces[i][2] - 1][0] * WindmillBodyScale, baseVertices[baseFaces[i][2] - 1][1] * WindmillBodyScale, baseVertices[baseFaces[i][2] - 1][2] * WindmillBodyScale);
        glEnd();
    }
}

void display() {
    if (takingInput == 0) {


        if (angle >= 2 * pi) {
            angle = -2 * pi;
        }
        /*else if (angle <= -16 * pi) {
            angle = 16 * pi;
        }*/
        float u = -angle / 20 / pi;
        if (fetched_and_called == 0) {
            fetched_and_called = 1;
            calculate_power_and_rpm();
            sprintf(current_place, "Location : %s, %s, %s", name_of_place, region, country);
        }
        //glClearColor(0.0, u, 1, 1.0);
        //printf("\nu = %f", u);
        // reset
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);
        xyTOxyz(screenWidth, screenHeight);
        glLoadIdentity();

        // move camera
        if (opened_up_view == 0) {
            gluLookAt(5 + 30 * sin(cameraHorizMovement * 3.1415 / 180.0), 2, -25 + 30 * cos(cameraHorizMovement * 3.1415 / 180.0), 0, 5, -20, 0, 1, 0);
        }
        else {
            gluLookAt(-5 + 5 * sin(cameraHorizMovement * 3.1415 / 180.0), 17, -25 + 5 * cos(cameraHorizMovement * 3.1415 / 180.0), 0, 5, -22, 0, 1, 0);
            //printf("%f\n", cameraHorizMovement);
        }
        initLight();
        // get updated values
        rotationMatrixUpdate();

        // All Objects Translations & Rotations
        glTranslatef(0, 0, -20); // move objects behind into the scene of projection.
        glRotatef(45, 0, 1, 0); // rotate all objects w.r.t y axis for 3D effect.

        // Windblade Translations & Rotations & Drawing
        glTranslatef(0, 10, 0); // move blades upwards in y w.r.t origin.


        glRotatef(angle, 0, 0, 1); // rotate w.r.t the current angle of blade
        glRotatef(90, 1, 0, 0); // rotate blade for correct orientation
        glTranslatef(-0.4, 0, 0); // move blade outward with respect to windmill body top
        drawBlade();
        glTranslatef(0.4, 0, 0); // move back to center of windmill body top
        glRotatef(-90, 1, 0, 0); // rotate back to original orientation

        glRotatef(120, 0, 0, 1); // rotate w.r.t the current angle of blade
        glRotatef(90, 1, 0, 0); // rotate blade for correct orientation
        glTranslatef(-0.4, 0, 0); // move blade outward with respect to windmill body top
        drawBlade();
        glTranslatef(0.4, 0, 0); // move back to center of windmill body top
        glRotatef(-90, 1, 0, 0); // rotate back to original orientation

        glRotatef(120, 0, 0, 1); // rotate w.r.t the current angle of blade
        glRotatef(90, 1, 0, 0); // rotate blade for correct orientation
        glTranslatef(-0.4, 0, 0); // move blade outward with respect to windmill body top
        drawBlade();
        glTranslatef(0.4, 0, 0); // move back to center of windmill body top
        glRotatef(-90, 1, 0, 0); // rotate back to original orientation

        glRotatef(120 - angle, 0, 0, 1); // come back to 0 rotation w.r.t z axis for drawing body.



        glTranslatef(0, -6.22, -0.8); // translate the space pointer below to point at base of windmill body
        drawTerrain();
        if (opened_up_view == 0) {
            //glClearColor(0.0, 1.0, 1, 1.0);
            drawBody();
        }
        else {
            drawInternals();
            glTranslatef(0, 6.22, -0.8);
            glRotatef(angle, 0, 0, 1);
            drawShaft();
            glRotatef(-1 * angle, 0, 0, 1);
            glTranslatef(0, -6.22, 0.8);

        }
        //drawBase();

        xyzTOxy(screenWidth, screenHeight);
        glLoadIdentity();
        //glColor3f(0, 0, 0);
        //renderBitmapString(screenWidth - 140, screenHeight - 75, GLUT_BITMAP_TIMES_ROMAN_24, "Results");

        format_time(current_time);
        glColor3f(2, 2, 0);
        renderBitmapString(5, screenHeight - 15, GLUT_BITMAP_HELVETICA_18, current_time);
        renderBitmapString(5, screenHeight - 35, GLUT_BITMAP_HELVETICA_18, current_place);
        renderBitmapString(5, screenHeight - 55, GLUT_BITMAP_HELVETICA_18, instantaneous_power_gen_string);
        if (displayTemp != NULL)
        {
            /*printf("\n\nTime interval: %s", displayTemp->nodeTitle);
            printf("\n\nPower: %.5f", displayTemp->power_gen);
            printf("\nrpm : %f", displayTemp->rpm);
            printf("\n-------------------------");*/
            //printf("\n00");
            renderBitmapString(5, screenHeight - 85, GLUT_BITMAP_HELVETICA_18, "Hyperlapse : ");
            //printf("\n11");
            renderBitmapString(5, screenHeight - 105, GLUT_BITMAP_HELVETICA_18, current_windmill_details_for_display_time_interval);

            renderBitmapString(5, screenHeight - 125, GLUT_BITMAP_HELVETICA_18, current_windmill_details_for_display_power);

            renderBitmapString(5, screenHeight - 145, GLUT_BITMAP_HELVETICA_18, current_windmill_details_for_display_rpm);
            currentFrame++;
            if (currentFrame >= 200) {
                if (displayTemp != NULL) {
                    displayTemp = displayTemp->next;
                    sprintf(current_windmill_details_for_display_time_interval, "    Time Interval : %s", displayTemp->nodeTitle);
                    sprintf(current_windmill_details_for_display_power, "    Power Generated : %.5f kWH", displayTemp->power_gen);
                    sprintf(current_windmill_details_for_display_rpm, "    Windmill RPM : %f", displayTemp->rpm);
                    currentFrame = 0;
                    angularSpeed = -pi * displayTemp->rpm / 180 * 3;
                }
            }
        }
        else {
            displayTemp = wHead;
        }
        /*glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
        glVertex2f(screenHeight - 50, screenWidth - 50);
        glVertex2f(screenHeight - 150, screenWidth - 50);
        glVertex2f(screenHeight - 150, screenWidth - 150);
        glVertex2f(screenHeight - 50, screenWidth - 150);
        glEnd();*/

        // flush or swap buffer
        glutSwapBuffers();
    }
    else {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
        //glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Enable texture mapping
        glEnable(GL_TEXTURE_2D);

        // Create a texture object
        GLuint texture;
        glGenTextures(1, &texture);

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, texture);

        // Upload the pixel data to the texture object
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tgaFile.width, tgaFile.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tgaFile.pixelData);

        // Set the texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        // Define the size of the quad relative to the window
        // Define the size of the quad relative to the window
        float quadSize = 1.0f;

        // Calculate the quad corners
        float left = -quadSize * tgaFile.width / tgaFile.height;
        float right = quadSize * tgaFile.width / tgaFile.height;
        float bottom = -quadSize;
        float top = quadSize;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(left, bottom);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(right, bottom);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(right, top);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(left, top);
        glEnd();



        // Disable texture mapping
        glDisable(GL_TEXTURE_2D);

        // Delete the texture object
        glDeleteTextures(1, &texture);

        // Free the pixel data
        //free(tgaFile.pixelData);

        glutSwapBuffers();
    }
}

void frameUpdate(int z) {
    // This function is used to fire re-display every frame after fixed interval of time by calling itself and the re-display function.
    glutPostRedisplay();
    glutTimerFunc((float)1000 / (float)240, frameUpdate, 0); // we call this function again after the time specified.
}


void getMouseCLicks(int button, int state, int x, int y) {
    //printf("%d %d %d\n", x, y, state);
    if (takingInput == 1) {
        if (state == GLUT_DOWN) {
            char file_name[1000] = "";
            if (chooseCSV(file_name) == 0) {
                return;
            }
            printf("%s\n---", file_name);

            read_csv(file_name);
            takingInput = 0;
            struct windmill* temp = wHead;
            glutMouseFunc(NULL);
        }

    }
}


void format_time(char* output) {  // from : https://stackoverflow.com/questions/5141960/get-the-current-time-in-c
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(output, "Current Time : %02d-%02d-%4d %02d:%02d:%02d", timeinfo->tm_mday,
        timeinfo->tm_mon + 1, timeinfo->tm_year + 1900,
        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void format_time_for_csv(char* output) {  // from : https://stackoverflow.com/questions/5141960/get-the-current-time-in-c
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(output, "%4d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}

void write_csv() {  // TODO : Total Day-wise and overall total
    char FileName[300] = "";
    OPENFILENAME  ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    wchar_t szFile[25 * MAX_PATH];
    wchar_t szPath[MAX_PATH];
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    //ofn.hInstance = NULL;
    ofn.lpstrFilter = L"CSV Files (.csv)\0*.csv\0\0";
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    //ofn.lpstrTitle = L"Please Select A File To Open";
    //ofn.nMaxFile = 1000;
    ofn.Flags = OFN_NONETWORKBUTTON | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    if (!GetSaveFileName(&ofn)) {
        return;
    }
    //printf("The selected file path is - %s\n", ofn.lpstrFile);
    /*for (int i = 0; i < 100; i++) {
        printf("\n%d %c", ofn.lpstrFile[i], ofn.lpstrFile[i]);
    }*/
    int i = 0;
    while (ofn.lpstrFile[i] != 0) {
        FileName[i] = ofn.lpstrFile[i];
        i++;
    }
    FileName[i] = '\0';
    int len = i;
    printf("\n\nFilename - %s", FileName);
    if (FileName[len - 4] != '.' || FileName[len - 3] != 'c' || FileName[len - 2] != 's' || FileName[len - 1] != 'v') {
        FileName[i] = '.';
        i++;
        FileName[i] = 'c';
        i++;
        FileName[i] = 's';
        i++;
        FileName[i] = 'v';
        i++;
        FileName[i] = '\0';
        len = i;
    }
    printf("\n\nFilename - %s", FileName);
    FILE* csv_FilePtr;
    csv_FilePtr = fopen(FileName, "w");

    fprintf(csv_FilePtr, "Hour-wise data\n\n\n");
    fprintf(csv_FilePtr, "Time,Power generated (kWH)\n");
    struct windmill* myDisplayTemp = wHead->next;
    while (myDisplayTemp != NULL) {
        fprintf(csv_FilePtr, "%s,%.5f\n", myDisplayTemp->nodeTitle, myDisplayTemp->power_gen);
        printf("%s,%.5f\n", myDisplayTemp->nodeTitle, myDisplayTemp->power_gen);
        myDisplayTemp = myDisplayTemp->next;
    }
    float power_sum = 0;
    myDisplayTemp = wHead->next;
    for (int i = 0; i < 24; i++) {
        power_sum += myDisplayTemp->power_gen;
        myDisplayTemp = myDisplayTemp->next;
    }
    fprintf(csv_FilePtr, "\n , \n , \n , \n , \n");
    fprintf(csv_FilePtr, "Day-wise data\n");
    fprintf(csv_FilePtr, "\n\nDay 1 power generated (kWH), %f\n", power_sum);
    float power_sum_2 = 0;
    for (int i = 0; i < 24; i++) {
        power_sum_2 += myDisplayTemp->power_gen;
        myDisplayTemp = myDisplayTemp->next;
    }
    fprintf(csv_FilePtr, "Day 2 power generated (kWH), %f\n", power_sum_2);
    float power_sum_3 = 0;
    for (int i = 0; i < 23; i++) {
        power_sum_3 += myDisplayTemp->power_gen;
        myDisplayTemp = myDisplayTemp->next;
    }
    fprintf(csv_FilePtr, "Day 3 power generated (kWH), %f\n", power_sum_3);
    fprintf(csv_FilePtr, "\n\nTotal power generated (kWH), %f\n", power_sum + power_sum_2 + power_sum_3);
    fprintf(csv_FilePtr, "\n , \n , \n , \n , \n , ");
    fprintf(csv_FilePtr, "\n\"Report Generated By\",\"Digital Windmill Twin?(Idk), VIT Pune\"\n");
    char curr_time[100];
    format_time_for_csv(current_time);
    fprintf(csv_FilePtr, "Time,%s\n", current_time);
    fprintf(csv_FilePtr, "\nGenerated for the windmill farm having following properties,\n");
    for (int i = 0; i < 12; i++) {
        fprintf(csv_FilePtr, "%s,%s\n", csv_keys[i], csv_values[i]);
    }
    fclose(csv_FilePtr);
    printf("File Successfully Saved!");
}

void keyPressed(unsigned char key, int x, int y) {
    if (takingInput == 1) {
        /*if (key == GLUT_LEFT_BUTTON) {
            char file_name[1000] = "";
            chooseCSV(file_name);
            printf("%s\n---", file_name);

            read_csv(file_name);
            displayTemp = wHead;
            takingInput = 0;
        }*/
        /*if (inputNumber >= 3) {
            takingInput = 0;
            printf("1212121212121221");
            return;
        }
        if (inputFilled >= 14) {
            inputNumber++;
            inputFilled = 0;
            if (inputNumber >= 3) {
                takingInput = 0;
                printf("33333333333333333333333331");
                return;
            }
        }
        if (key == 13 || key == 10) { // Enter key
            inputNumber++;
            inputFilled = 0;
            printf("5555555521221");
            if (inputNumber >= 3) {
                takingInput = 0;
                printf("444444444444421221");
                return;
            }
        }
        if (key == 8) { // backspace
            if (inputFilled > 0) {
                input[inputNumber][inputFilled] = '\0';
                inputFilled--;
            }
            return;
        }
        input[inputNumber][inputFilled] = key;
        printf("\nInput = %s %s %s %d %d", input[0], input[1], input[2], inputFilled, inputNumber);
        /*for (int i = 0; i < 14; i++) {
            printf("%d|", input[i]);
        }*/
        /*inputFilled += 1; */
    }
    else {
        if (key == 'a') {
            //printf("Left");
            cameraHorizMovement -= 1;
        }
        else if (key == 'd') {
            //printf("Right");
            cameraHorizMovement += 1;
        }
        else if (key == 'w') {
            //printf("Up");
            cameraVertMovement += 0.1;
        }
        else if (key == 's') {
            printf("Down");
            write_csv();
        }
        else if (key == 'o') {
            if (opened_up_view == 0) {
                printf("\nOpenedUpView");
                opened_up_view = 1;
            }
            else {
                printf("\nNotOpenedUpView");
                opened_up_view = 0;
            }
        }
    }
}

char* call_api() {
    // Initialize session
    HINTERNET session = WinHttpOpen(L"WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify server and port
    HINTERNET connect = WinHttpConnect(session, L"api.weatherapi.com",
        INTERNET_DEFAULT_PORT, 0);

    /*char x_coordinate[100];
    printf("\n\nEnter X-coordinate : ");
    scanf("%s", x_coordinate);
    char y_coordinate[100];
    printf("Enter Y-coordinate : ");
    scanf("%s", y_coordinate);*/

    // Create an HTTP request handle
    //"/v1/forecast.json?key=4d37b03c649f4b41beb184538230508&q=19.0760,72.8777&days=3"
    char* webAddress = malloc(strlen("/v1/forecast.json?key=4d37b03c649f4b41beb184538230508&q=") + strlen(csv_values[0]) + strlen(",") + strlen(csv_values[1]) + strlen("&days=3") + 100); // +1 for the null terminator
    //char* webAddress = malloc(strlen("/v1/forecast.json?key=4d37b03c649f4b41beb184538230508&q=") + strlen(x_coordinate) + strlen(",") + strlen(y_coordinate) + strlen("&days=3") + 100); // +1 for the null terminator
    strcpy(webAddress, "/v1/forecast.json?key=4d37b03c649f4b41beb184538230508&q=");
    strcat(webAddress, csv_values[0]);
    //strcat(webAddress, x_coordinate);
    strcat(webAddress, ",");
    strcat(webAddress, csv_values[1]);
    //strcat(webAddress, y_coordinate);
    strcat(webAddress, "&days=3");
    strcat(webAddress, "\0");
    printf("\n%s\n%s\n%s\n", webAddress, csv_values[0], csv_values[1]);
    char webAddressToSend[1000];
    mbstowcs(webAddressToSend, webAddress, 1000);  // Convert to wchar_t s
    HINTERNET request = WinHttpOpenRequest(connect, L"GET", webAddressToSend,
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0);

    // Send a request
    BOOL result = WinHttpSendRequest(request,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0,
        0, 0);

    // Wait for the response
    result = WinHttpReceiveResponse(request, NULL);

    // Read the data
    DWORD size = 0;
    DWORD downloaded = 0;
    LPSTR buffer = NULL;
    LPSTR total_buffer = NULL; // Declare a pointer to store the total buffer
    do {
        // Check for available data
        result = WinHttpQueryDataAvailable(request, &size);
        if (!result) break;

        // Allocate space for the buffer
        buffer = (LPSTR)malloc(sizeof(char) * size + 1);
        if (!buffer) break;

        // Read the data
        result = WinHttpReadData(request, (LPVOID)buffer, size, &downloaded);
        if (!result) break;

        // Null-terminate the buffer
        buffer[downloaded] = '\0';

        // Concatenate the buffer to the total buffer using strcat function
        if (total_buffer == NULL) { // If this is the first buffer, allocate memory for total buffer
            total_buffer = (LPSTR)malloc(sizeof(char) * (downloaded + 1));
            if (total_buffer == NULL) break;
            strcpy(total_buffer, buffer); // Copy the buffer to the total buffer
        }
        else { // If this is not the first buffer, reallocate memory for total buffer and append the buffer
            total_buffer = (LPSTR)realloc(total_buffer, sizeof(char) * (strlen(total_buffer) + downloaded + 1));
            if (total_buffer == NULL) break;
            strcat(total_buffer, buffer); // Append the buffer to the total buffer
        }

        // Free the buffer
        free(buffer);
        buffer = NULL;
    } while (size > 0);

    // Close the handles
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    printf("%s", total_buffer);


    return total_buffer;
}
