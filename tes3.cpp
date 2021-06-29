#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <cassert>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <climits>
#include <cstdlib>
#include <ctime>
#include <algorithm>

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};


int player;
const int SIZE = 8;
//std::array<std::array<int, SIZE>, SIZE> board;
//std::vector<Point> next_valid_spots;

int pointW[8][8]={
    {100,-25,10, 5, 5,10,-25,100},
    {-25,-25, 1, 1, 1, 1,-25,-25},
    { 10,  1, 5, 2, 2, 5,  1, 10},
    {  5,  1, 2, 1, 1, 2,  1,  5},
    {  5,  1, 2, 1, 1, 2,  1,  5},
    { 10,  1, 5, 2, 2, 5,  1, 10},
    {-25,-25, 1, 1, 1, 1,-25,-25},
    {100,-25,10, 5, 5,10,-25,100}
};

int scoreW[8][8]={
    { 8, 2, 2, 2, 2, 2, 1, 8},
    { 2, 1, 1, 1, 1, 1, 1, 2},
    { 2, 1, 2, 2, 2, 2, 1, 2},
    { 2, 1, 2, 1, 1, 2, 1, 2},
    { 2, 1, 2, 1, 1, 2, 1, 2},
    { 2, 1, 2, 2, 2, 2, 1, 2},
    { 2, 1, 1, 1, 1, 1, 1, 2},
    { 8, 2, 2, 2, 2, 2, 2, 8}
};

////////////////////////////////////////////////////////////////////////////////////
class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;

    double scorePlacement;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        this->scorePlacement=scoreW[center.x][center.y];// cek2820
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    /*
    OthelloBoard(const std::array<std::array<int, SIZE>, SIZE> board,int play){
    	for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                this->board[i][j] = board[i][j];
                disc_count[board[i][j]]++;
            }
        }
        cur_player=play;
        next_valid_spots=this->get_valid_spots();
    }
    */
    OthelloBoard& operator= (const OthelloBoard& rhs){
    	for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                this->board[i][j] = rhs.board[i][j];
            }
        }
        cur_player = rhs.cur_player;
        disc_count[EMPTY] = rhs.disc_count[EMPTY];
        disc_count[BLACK] = rhs.disc_count[BLACK];
        disc_count[WHITE] = rhs.disc_count[WHITE];
        for(auto it:rhs.next_valid_spots) next_valid_spots.emplace_back(it);
        done = rhs.done;
        winner = rhs.winner;
        return *this;
    }
    bool operator==(const OthelloBoard& rhs){
    	for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if(this->board[i][j] != rhs.board[i][j])return false;
            }
        }
        return true;
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        disc_count[EMPTY] = 0;
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
    int heuristicAbs(int p){
        int i=0,sz=SIZE-1,score=0;
        bool cek[8]={true,true,true,true,true,true,true,true};
    	//std::fill(cek,cek+sizeof(cek),true);
        while(cek[0]||cek[1]||cek[2]||cek[3]||cek[4]||cek[5]||cek[6]||cek[7]){
            if(i==SIZE)break;
            if(cek[0]&&board[0][i]==p){
                score++;
            }else cek[0]=false;
            if(cek[1]&&board[i][0]==p){
                score++;
            }else cek[1]=false;

            if(cek[2]&&board[0][sz-i]==p){
                score++;
            }else cek[2]=false;
            if(cek[3]&&board[i][sz]==p){
                score++;
            }else cek[3]=false;

            if(cek[4]&&board[sz][sz-i]==p){
                score++;
            }else cek[4]=false;
            if(cek[5]&&board[sz-i][sz]==p){
                score++;
            }else cek[5]=false;

            if(cek[6]&&board[sz][i]==p){
                score++;
            }else cek[6]=false;
            if(cek[sz]&&board[sz-i][0]==p){
                score++;
            }else cek[7]=false;
            i++;
    	}
    	return score;
    }
    //int heuristicPoint(int p){
    //	return 0;//cek2814 kasi angka
    //}
    int heuristicValue(int p){
    	int score=0;
    	for(int i=0;i<SIZE;i++){
            for(int j=0;j<SIZE;j++){
                if(p==board[i][j]){
                    score+=pointW[i][j];
                }else if((3-p)==board[i][j]){
                    score-=pointW[i][j];
                }
            }
    	}
    	return score;
    }
};
/////////////////////////////////////////////////////////////////////////////new///
OthelloBoard root;


float valueBoard(OthelloBoard now){
    float score;
    score=now.heuristicValue(player)+3*((player==now.cur_player)?now.scorePlacement:-now.scorePlacement);
    score+=(now.heuristicAbs(player)<<2);
    score-=(now.heuristicAbs(3-player)<<2);
    return score;
}

Point minimax(OthelloBoard now,int deep);
float maxim(OthelloBoard now, int deep, float a, float b);
float minim(OthelloBoard now, int deep, float a, float b);

Point minimax(OthelloBoard now,int deep){
    std::cout<<"PLAYERs:"<<player<<"\n";
    Point best(-1,-1);
    OthelloBoard next;
    int value = INT_MIN, tmp;
    for(auto it:root.next_valid_spots){
        std::cout<<it.x<<","<<it.y<<"\n";
    }

    int i=0;
    for(auto it:root.next_valid_spots){
        std::cout<<":.:.:.:.:.:.:.:.:.:: "<<it.x<<","<<it.y<<"::.:.:.:.:.:.:.:.:.:.:.:.:.:.:.:\n";
        next=now;
        next.put_disc(it);
        if(next.cur_player!=player){
            std::cout<<"works1\n";
            tmp = minim(next,deep-1,INT_MIN,INT_MAX);
        }else{
            std::cout<<"works2\n";
            tmp = maxim(next,deep-2,INT_MIN,INT_MAX);
        }
        std::cout<<"["<<tmp<<"]";
        if(value<tmp){
            best=it;
            value=tmp;
            std::cout<<"DAPETTTT"<<best.x<<best.y<<"\n";
        }else std::cout<<"NOPE"<<best.x<<best.y<<"\n";
    }
    return best;
}

float maxim(OthelloBoard now, int deep, float a, float b){
    std::cout<<"max"<<deep<<" ";

    float value=INT_MIN;
    if(deep<=0||now.done){
        std::cout<<">>"<<deep<<"(P"<<now.cur_player<<"-s:";//debug
        value=valueBoard(now);
        std::cout<<"):";//debug
        return value;
    }else{
        OthelloBoard next;
        if(now.next_valid_spots.empty())std::cout<<"empty ";//debug
        for(auto it:now.next_valid_spots){
            next=now;
            next.put_disc(it);
            if(now.cur_player==next.cur_player)
                value=std::max(maxim(next,deep-2,a,b),value);
            else
                value=std::max(minim(next,deep-1,a,b),value);
        }
        //std::cout<<">>"<<deep<<"(P"<<now.cur_player<<"-s:"<<value<<"):";
        return value;
    }
    std::cout<<"ESCAPED\n";//debug
}

float minim(OthelloBoard now, int deep, float a, float b){
    std::cout<<"min"<<deep<<" ";//debug

    float value=INT_MAX;

    if(deep<=0||now.done){
        //std::cout<<">>"<<deep<<"(P"<<now.cur_player<<")"<<"-s:";//debug
        value=valueBoard(now);
        //std::cout<<"):";//debug
        return value;
    }else{
        if(now.next_valid_spots.empty())std::cout<<"empty ";//debug
        OthelloBoard next;
        for(auto it:now.next_valid_spots){
            next=now;
            next.put_disc(it);
            if(now.cur_player!=next.cur_player)
                value=std::min(maxim(next,deep-1,a,b),value);
            else
                value=std::min(minim(next,deep-2,a,b),value);
        }
        //std::cout<<">>"<<deep<<"(P"<<now.cur_player<<"-s:"<<value<<"):";
        return value;
    }
    std::cout<<"ESCAPED\n";
}

//////////////////////////////////////////////////////////////////////////////////

void read_board(std::ifstream& fin) {
    fin >> player;
    std::cout<<"PLAYER: "<<player<<"\n";//debug
    root.cur_player=player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> root.board[i][j];
        }
    }
}
void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    std::cout<<"INIT: ";
    root.next_valid_spots.clear();
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        root.next_valid_spots.push_back({(float) x,(float) y});
        //std::cout<<x<<y<<"\n";//debug
    }
}

void write_valid_spot(std::ofstream& fout) {
    //int n_valid_spots = root.next_valid_spots.size();
    //srand(time(NULL));
    // Choose random spot. (Not random uniform here)
    //int index = (rand() % n_valid_spots);
    Point p = minimax(root,2);
    std::cout<<" >>I choose: ("<<p.x<<", "<<p.y<<")\n";//debug
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}


int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

