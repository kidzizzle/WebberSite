#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>


#define TAGLISTMAX 10
#define TAGLISTMAXSTRINGLENGTH 20
#define REVIEWNAVPATHTOREVIEWS "./reviews/"

typedef enum {BLOG, MUSIC, MOVIE, VG, GENERAL} listType;

// Some type of struct to hold the listing type info
typedef struct
{
    char* directoryPath;
    char* navDirectoryPath;
    char* titleDecoration;
    char* titleDecorationCloser;
    char* titleStyles;
    char* descDecoration;
    char* descDecorationCloser;
    char* descStyles;
} listTypeValues; 

void destroyListTypeValues(listTypeValues *t)
{
    free(t->directoryPath);
    free(t->navDirectoryPath);
    free(t->titleDecoration);
    free(t->titleDecorationCloser);
    free(t->titleStyles);
    free(t->descDecoration);
    free(t->descDecorationCloser);
    free(t->descStyles);
    free(t);
    t = NULL;
}

listTypeValues *createListTypeValues(const char* directoryPath, const char* navDirectoryPath, const char* titleDecoration, const char* titleDecorationCloser, const char* titleStyles, const char* descDecoration, const char* descDecorationCloser, const char* descStyles)
{
    listTypeValues *t = malloc(sizeof(listTypeValues));

    t->directoryPath = strdup(directoryPath);
    t->navDirectoryPath = strdup(navDirectoryPath);
    t->titleDecoration = strdup(titleDecoration);
    t->titleDecorationCloser = strdup(titleDecorationCloser);
    t->titleStyles = strdup(titleStyles);
    t->descDecoration = strdup(descDecoration);
    t->descDecorationCloser = strdup(descDecorationCloser);
    t->descStyles = strdup(descStyles);

    return t;
}

typedef struct
{
    char* fileName;
    char* title;
    char* description;
    char* tag;
} postListing;

void destroyPostListing(postListing *t)
{
    free(t->fileName);
    free(t->title);
    free(t->description);
    free(t->tag);
    free(t);
    t = NULL;
}

postListing *createPostListing(const char* fileName, const char* title, const char* description, const char* tag) 
{
    postListing *t = malloc(sizeof(postListing));

    t->fileName = strdup(fileName);
    t->title = strdup(title);
    t->description = strdup(description);
    t->tag = strdup(tag);

    return t;
}

listTypeValues *populateListInfo(listType typeOfList)
{
    listTypeValues *t;

    printf("Populating List Info\n");

    // fill in info for listTypeValues
    switch (typeOfList) {
        case BLOG:
            t = createListTypeValues("../pages/blogs/", "./blogs/", "<b>", "</b>", "", "&ensp;", "", "");
            break;
        case MUSIC:
            t = createListTypeValues("../pages/reviews/music/", "./music/", "<b>", "</b>", "color: yellow;", "&ensp;", "", ""); 
            break;
        case MOVIE:
            t = createListTypeValues("../pages/reviews/movietv/", "./movietv/", "<b>", "</b>", "color: yellow;", "&ensp;", "", "");
            break;
        case VG:
            t = createListTypeValues("../pages/reviews/videogames/", "./videogames/", "<b>", "</b>", "color: yellow;", "&ensp;", "", "");
            break;
        default:
            break;
    } 

    return t;
}

postListing *parsePost(char* directoryPath, char* fileName)
{
    char* filePath;
    filePath = malloc(strlen(directoryPath) + strlen(fileName) + 1);
    strcpy(filePath, directoryPath);
    strcat(filePath, fileName);
    printf("Parsing: %s\n", filePath);

    char* title = NULL;
    char* description = NULL;
    char* tag = NULL;
    FILE* postFile = fopen(filePath, "r");
    char lineBuffer[512];

    while (fgets(lineBuffer, sizeof(lineBuffer), postFile)) {
        if (strstr(lineBuffer, "<!--TITLE") != NULL) {
            title = malloc(strlen(lineBuffer));
            if (sscanf(lineBuffer, "%*[^\"]\"%100[^\"]\"", title) != 1) {
                printf("Error in title field for file: %s\n", fileName);
                exit(1);
            }
            
        } else if (strstr(lineBuffer, "<!--DESCRIPTION") != NULL) {
            description = malloc(strlen(lineBuffer));
            if (sscanf(lineBuffer, "%*[^\"]\"%100[^\"]\"", description) != 1) {
                printf("Error in description field for file: %s\n", fileName);
                exit(1);
            }

        } else if (strstr(lineBuffer, "<!--TAGS") != NULL) {
            tag = malloc(strlen(lineBuffer));
            if (sscanf(lineBuffer, "%*[^\"]\"%100[^\"]\"", tag) != 1) {
                printf("Error in tag field for file: %s\n", fileName);
                exit(1);
            }

        }
    }

    fclose(postFile);
    postListing* t = createPostListing(fileName, title, description, tag);

    free(title);
    free(description);
    free(tag);
    return t;
}

bool checkForTag(char tagList[TAGLISTMAX][TAGLISTMAXSTRINGLENGTH], char* tag)
{
    bool doesTagExist = false; 
    for (int i = 0; i < TAGLISTMAX; i++) {
        if (strcmp(tagList[i], tag) == 0) {
            doesTagExist = true;
            return doesTagExist;
        }
    } 

    return doesTagExist;
}

// Function to generate listings
void genNewNavFile(FILE* outputFile, listType typeOfList, bool inGeneralNav)
{

    listTypeValues *listInfo;
    postListing *currPost;
    char tagList[TAGLISTMAX][TAGLISTMAXSTRINGLENGTH];
    int numberOfTags = 0;

    postListing *allPosts[50]; 
    int numberOfPosts = 0;

    // Get all titles, desc, and filepaths of appropriate posts
    listInfo = populateListInfo(typeOfList); 

    printf("%s\n", listInfo->directoryPath);

    // Open folder for posts
    struct dirent *de;
    DIR* dr = opendir(listInfo->directoryPath);
    if (dr == NULL) {
        printf("Couldn't open directory");
        return;
    }
    
    // Iterate through posts
    while ((de = readdir(dr)) != NULL) {
        int len = strlen(de->d_name);
        char extCheck[5];
        for (int i = len - 5; i < len; i++) {
            extCheck[i - len + 5] = de->d_name[i];
        }
        // TODO: Figure out how to generate tag for music nav
        // TODO: General Review logistics
        if (strstr(extCheck, ".html") != NULL) {
            // de->d_name = filename of post
            currPost = parsePost(listInfo->directoryPath, de->d_name);

            if (currPost->tag != NULL) {
                if(!checkForTag(tagList, currPost->tag)) {
                    printf("NEW TAG!!! %s\n", currPost->tag);
                    strcpy(tagList[numberOfTags], currPost->tag);
                    numberOfTags++;
                }
            }

            allPosts[numberOfPosts] = currPost;
            numberOfPosts++;
        }
    }

    printf("Number of posts: %i\n", numberOfPosts);

    if (inGeneralNav) {
        fprintf(outputFile, "\t<br>\n");
        switch (typeOfList) {
            case MOVIE:
                fprintf(outputFile, 
                        "\t<h3 class=\"center\" id=\"moviesection\">Movie and TV Reviews</h3>\n");
                fprintf(outputFile,
                        "\t<a href=\"%smovietvreviewnav.html\"><p class=\"center\"><em>Click here to see all movie and tv reviews...</em></p></a>\n", REVIEWNAVPATHTOREVIEWS );
                break;
            case MUSIC:
                fprintf(outputFile, 
                        "\t<h3 class=\"center\" id=\"musicsection\">Music Reviews</h3>\n");
                fprintf(outputFile,
                        "\t<a href=\"%smusicreviewnav.html\"><p class=\"center\"><em>Click here to see all music reviews...</em></p></a>\n", REVIEWNAVPATHTOREVIEWS );
                break;
            case VG:
                fprintf(outputFile, 
                        "\t<h3 class=\"center\" id=\"videogamesection\">Video Game Reviews</h3>\n");
                fprintf(outputFile,
                        "\t<a href=\"%svideogamereviewnav.html\"><p class=\"center\"><em>Click here to see all video game reviews...</em></p></a>\n", REVIEWNAVPATHTOREVIEWS );
            case BLOG:
            default:
                break;
        }
        fprintf(outputFile, "\t<br>\n");
        int i = 0;
        if ((numberOfPosts - 10) > 0) {
            i = numberOfPosts - 10;
        } 
        for (; i < numberOfPosts; i++) {
            fprintf(outputFile, "\t\t<a href=\"%s%s%s\" style=\"color: yellow\"><b>%s</b></a>\n",
                    REVIEWNAVPATHTOREVIEWS, listInfo->navDirectoryPath, allPosts[i]->fileName, 
                    allPosts[i]->title);
            fprintf(outputFile, "\t\t<p>&ensp;%s</p>\n", allPosts[i]->description);
        }
    } else if (numberOfTags > 0) {
        for (int i = 0; i < numberOfTags; i++) {
            printf("Posts tagged %s:\n", tagList[i]);
            fprintf(outputFile, "\t<h3 id=\"%s\">%s</h3>\n",
                    tagList[i], tagList[i]);
            for (int j = numberOfPosts - 1; j >= 0; j--) {
                if (strcmp(allPosts[j]->tag, tagList[i]) == 0) {
                    fprintf(outputFile, "\t<a href=\"%s%s\" style=\"%s\">%s%s%s</a>\n", 
                        listInfo->navDirectoryPath, allPosts[j]->fileName, listInfo->titleStyles, 
                        listInfo->titleDecoration, allPosts[j]->title, listInfo->titleDecorationCloser);
                    fprintf(outputFile, "\t<p style=\"%s\">%s%s%s</p>\n",
                        listInfo->descStyles, listInfo->descDecoration, allPosts[j]->description, 
                        listInfo->descDecorationCloser);
                    printf("\t%s: %s\n", allPosts[j]->title, allPosts[j]->tag);
                }
            }
        }
    } else {
        for (int i = numberOfPosts - 1; i >= 0; i--) {
            fprintf(outputFile, "\t<a href=\"%s%s\" style=\"%s\">%s%s%s</a>\n", 
                listInfo->navDirectoryPath, allPosts[i]->fileName, listInfo->titleStyles, 
                listInfo->titleDecoration, allPosts[i]->title, listInfo->titleDecorationCloser);
            fprintf(outputFile, "\t<p style=\"%s\">%s%s%s</p>\n",
                listInfo->descStyles, listInfo->descDecoration, allPosts[i]->description, 
                listInfo->descDecorationCloser);
        }
    }

    for (int i = 0; i < numberOfPosts; i++) {
        destroyPostListing(allPosts[i]);
    }

    closedir(dr);
    destroyListTypeValues(listInfo);

    // Output listings based on information
}

// Function to read file up to <!-- itemList -->
// and output listings
void updateNavFile(const char* navFileName, listType typeOfList)
{
    
    bool inGeneralNav = false;
    printf("\nUpdating:\n\t");

    switch (typeOfList) {
        case MUSIC:
            printf("Music Nav\n");
            break;
        case MOVIE:
            printf("Movie/TV Nav\n");
            break;
        case VG:
            printf("Videogame Nav\n");
            break;
        case BLOG:
            printf("Blog Nav\n");
            break;
        default:
            printf("General Nav\n");
            inGeneralNav = true;
            break;
    }

    char linebuffer[256];
    FILE* navFile = fopen(navFileName, "r");
    FILE* tempFile = fopen("temp.html", "w");
    bool inItemList = false;

    while (fgets(linebuffer, sizeof(linebuffer), navFile)) {
        if (strstr(linebuffer, "<!--itemlistover-->\n") != NULL) {
            inItemList = false;
        }

        if (inItemList) {
            continue;
        } else {
            fprintf(tempFile, "%s", linebuffer);
        }

        if (strstr(linebuffer, "<!--itemlist-->\n") != NULL) {
            inItemList = true; 
            if (inGeneralNav) {
                typeOfList = MUSIC;
                genNewNavFile(tempFile, typeOfList, true);
                typeOfList = MOVIE;
                genNewNavFile(tempFile, typeOfList, true);
                typeOfList = VG;
                genNewNavFile(tempFile, typeOfList, true);
            } else {
                genNewNavFile(tempFile, typeOfList, false);
            }
        }
    }

    fclose(navFile);
    fclose(tempFile);
    remove(navFileName);
    rename("./temp.html", navFileName);
}

int main(void)
{
    listType currListGen = BLOG;
    updateNavFile("../pages/blognav.html", currListGen);
    currListGen = MOVIE;
    updateNavFile("../pages/reviews/movietvreviewnav.html", currListGen);
    currListGen = VG;
    updateNavFile("../pages/reviews/videogamereviewnav.html", currListGen);
    currListGen = MUSIC;
    updateNavFile("../pages/reviews/musicreviewnav.html", currListGen);
    currListGen = GENERAL;
    updateNavFile("../pages/reviewnav.html", currListGen);
    return 0;
}
