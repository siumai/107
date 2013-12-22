#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include "imdb.h"
#include "path.h"
using namespace std;

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}


bool generateShortestPath(const imdb& db, const string& source, const string& target) {
    list<path> partialPaths;
    set<string> previouslySeenActors;
    set<film> previouslySeenFilms;
    
    path ppath(source);
    partialPaths.push_back(ppath);
    
    while (!partialPaths.empty() && partialPaths.front().getLength()<=5) {
        path apath = partialPaths.front();
        partialPaths.pop_front();
        string lastplayer = apath.getLastPlayer();
        cout << "lastplayer: " << lastplayer << endl;
        vector<film> credits;
        db.getCredits(lastplayer, credits);
        for (int i=0; i<credits.size(); i++) {
            film fm = credits[i];
            cout << "credit: " << fm.title << endl;
            
            set<film>::iterator it = previouslySeenFilms.find(fm);
            if (it==previouslySeenFilms.end()) {
                previouslySeenFilms.insert(fm);
                vector<string> cast;
                db.getCast(fm, cast);
                for (int j=0; j<cast.size(); j++) {
                    string player = cast[j];
                    cout << "player: " << player << endl;
                    set<string>::iterator strit = previouslySeenActors.find(player);
                    if (strit==previouslySeenActors.end()) {
                        previouslySeenActors.insert(player);
                        path clonedpath = apath;
                        clonedpath.addConnection(fm, player);
                        if (player == target) {
                            cout << clonedpath << endl;
                            return true;
                        } else {
                            partialPaths.push_back(clonedpath);
                        }
                    }
                }
            }
        }
    }
    return false;
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, const char *argv[])
{
  imdb db(determinePathToData(argv[1])); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    exit(1);
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      // replace the following line by a call to your generateShortestPath routine...
//        path p1(source);
//        film f1;
//        f1.title = "Siumaicat";
//        f1.year = 1990;
//        p1.addConnection(f1, "siumai");
//        cout << "path1: " << p1 << endl;
//        path p2(p1);
//        film f2;
//        f2.title = "Hargaucat";
//        f2.year = 1991;
//        p1.addConnection(f2, "hargau");
//        cout << "path2: " << p2 << endl;
//        cout << "path1: " << p1 << endl;

        bool isConnected = generateShortestPath(db, source, target);
        if (!isConnected) {
            cout << endl << "No path between those two people could be found." << endl << endl;
        }
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}

