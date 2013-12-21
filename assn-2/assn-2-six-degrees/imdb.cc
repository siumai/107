using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
  
    numActors = *(int *)actorFile;
    numMovies = *(int *)movieFile;
//    cout << "numActors: " << numActors << endl;
//    cout << "numMovies: " << numMovies << endl;
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

struct keyComponent {
    const void *keyElem;
    const void *arr;
} newKey;

int cmpPlayers(const void* one, const void* two) {
    char *player1 = *(char **)one;//(char *)(*((keyComponent *)one)).keyElem;
    int offset = *(int *)two;
    
    //printf("%s", player1);
    //printf("%d", offset);
    
    //return 0;
    
    char *aRecord2 = (char *)(*((keyComponent *)one)).arr + offset;

    return strcmp(player1, aRecord2);
}

int cmpFilms(const void* filmp1, const void* filmp2) {
    return 0;
}
                                                            
int *findElem(const void* key, const void* base, size_t num, int(*cmpfnc)(const void*, const void*)) {
    //printf("%s", (char *)(((keyComponent *)key)->keyElem));
    //printf("%d", *(int *)(((keyComponent *)key)->arr));
    
    void* rbase = (void*)((char*)base + sizeof(int));
    
    int* find = (int*) bsearch(key, rbase, num, sizeof(int), cmpfnc);

    return find;
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
    keyComponent keyComp;
    keyComp.keyElem = player.c_str();
    keyComp.arr = actorFile;
    
//    newKey.keyElem = player.c_str();
//    newKey.arr = actorFile;
    
    int *found = findElem((void*)&keyComp, actorFile, numActors, cmpPlayers);
    //printf("%d", *found);
    
    if(found == NULL) {
        return false;
    } else {
        char *actorRecord = (char *)actorFile + *found;
        //printf("%s \n", actorRecord);
        
        int byteNameSize = strlen(actorRecord) + 1;//player.size()+1;
        actorRecord += byteNameSize;
        if(byteNameSize%2!=0){
            byteNameSize++;
            actorRecord+=1;
        }
        
        short numPlayerFilms = *(short *)actorRecord;
        //cout<<"num movies: "<< numPlayerFilms<<endl;
        
        actorRecord+=2;
        
        if ((byteNameSize+2)%4 != 0) {
            actorRecord+=2;
        }
        
        for (int j=0; j<numPlayerFilms; j++) {
            int movieOffset = *(int *)(actorRecord+(4*j));
            char *movieRecord = (char *)movieFile + movieOffset;
            film currentFilm = getFilm(movieRecord);
            //cout << currentFilm.title << " " << currentFilm.year << endl;
            films.push_back(currentFilm);
        }
        
        return true;

    }
    
//    for (int i=0; i<numActors; i++) {
//        int offset = *(((int *)actorFile)+1+i);
//        char *actorRecord = (char *)actorFile + offset;
//               
//        if (strcmp(player.c_str(), actorRecord)==0) {
//            
//            int byteNameSize = player.size()+1;
//            actorRecord += byteNameSize;
//            if(byteNameSize%2!=0){
//                byteNameSize++;
//                actorRecord+=1;
//            }
//            
//            short numPlayerFilms = *(short *)actorRecord;
//            actorRecord+=2;
//            
//            if ((byteNameSize+2)%4 != 0) {
//                actorRecord+=2;
//            }
//            
//            for (int j=0; j<numPlayerFilms; j++) {
//                int movieOffset = *(int *)(actorRecord+(4*j));
//                char *movieRecord = (char *)movieFile + movieOffset;
//                film currentFilm = getFilm(movieRecord);
//                //cout << currentFilm.title << " " << currentFilm.year << endl;
//                films.push_back(currentFilm);
//            }
//            
//            return true;
//        }
//    }
//    
//    return false;

}


bool imdb::getCast(const film& movie, vector<string>& players) const {
    
    for (int i=0; i<numMovies; i++) {
        int offset = *(((int *)movieFile)+1+i);
        char *movieRecord = (char *)movieFile + offset;
        film cfilm = getFilm(movieRecord);
        if (cfilm == movie) {
            int movieNameYearBytes = strlen(movieRecord) + 2;
            movieRecord += movieNameYearBytes;
            if (movieNameYearBytes%2!=0) {
                movieNameYearBytes++;
                movieRecord+=1;
            }
            
            short numFilmPlayers = *(short *)movieRecord;
            movieRecord+=2;
            
            if ((movieNameYearBytes+2)%4 != 0) {
                movieRecord+=2;
            }
            
            for (int j=0; j<numFilmPlayers; j++) {
                int playerOffset = *(int *)(movieRecord+(4*j));
                char *actorRecord = (char *)actorFile + playerOffset;
                string aname(actorRecord);
                players.push_back(aname);
            }
            return true;
        }
        
    }
    return false;

}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

string imdb::getActor(const char *actorRecord) const {
    
    return "";
}

film imdb::getFilm(const char *movieRecord) const {
    int movieNameBytes = strlen(movieRecord) + 1;
    int yearDiff = *(movieRecord+movieNameBytes);
    
    film currentFilm;
    currentFilm.title.assign(movieRecord);
    currentFilm.year = yearDiff+1900;
    return currentFilm;
}



// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}


