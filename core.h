#include <iostream>
#include <cmath>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>
#include <fstream>

#define PI 3.14159265358979323846

float rand_gauss()
{
    float u1 = (float)rand() / RAND_MAX;
    float u2 = (float)rand() / RAND_MAX;
    return sqrt(-2 * log(u1)) * cos(2 * PI * u2);
}

float randnorm(int mean, int std)
{
    return rand_gauss() * std + mean;
}

const int cols = 512, rows = 64;

int ecp_bit_num[4] = {0, 11, 21, 31};
std::vector<int64_t> extra;

using namespace std;

struct randomizer {
    virtual int rand() = 0;
};

struct random2 : randomizer {
    float percent;
    int high;
    int low;
    int times;
    int *mapping;

    random2(float p, int h, int l, int t, bool random_mapping) {
        percent = p;
        high = h;
        low = l;
        times = t;
        int n = (high + low) * times;
        mapping = new int[n];
        for (int i = 0; i < n; ++i) mapping[i] = i;

        if (random_mapping) {
            random_shuffle(mapping, mapping + n);
        }
    }

    ~random2() {
        delete mapping;
    }

    int rand() {
        return (::rand() % times) * (high + low) + 
            (::rand() < RAND_MAX * percent ? ::rand() % high : high + ::rand() % low);
    }
};

struct lsbrand : randomizer {
    int group = 64;

    int rand() {
        int x = 0;
        while (::rand() % 2 && x < 7) ++x;
        return x + (::rand() % group) * 8;
    }
};

struct filerand : randomizer {
    vector<int> rows;
    int i;
    filerand() {
        ifstream ifs("rand.txt");
        int r;
        while (ifs >> r) {
            rows.push_back(r);
        }
        i = 0;
    }

    int rand() {
        if (i >= rows.size()) i = 0;
        return rows[i++];
    }
};

struct filerand_new : randomizer {
    vector<float> ps;
    filerand_new(const char *filename) {
        ifstream ifs(filename);
        float p;
        float psum = 0;
        ps.push_back(psum);
        while (ifs >> p) {
            psum += p;
            ps.push_back(psum);
        }
    }

    int rand() {
        // use bisection to find the right place to insert the new number
        float t = (float)::rand() / RAND_MAX;
        int l = 0, r = ps.size() - 1;
        while (l < r) {
            int m = (l + r) / 2;
            if (ps[m] > t) r = m;
            else l = m + 1;
        }
        return max(0, min(63, l - 1));
    }
};

struct config {
    const char* savefile;
#ifdef row_sparing
    int rs_count = 1;
#endif

#ifdef lsb
    lsbrand col_rand = lsbrand();
#else
    randomizer* col_rand = new random2(1, 100, 412, 1, false);
    randomizer* row_rand = new random2(1, 1, 63, 1, false);
#endif

    int aux = 3;
    float swap_endurance = 1;
    int mean = 100000;
    float cov = 0.3;
    int start_fault = 0;
    int crit_fault_every = 0;

#ifdef global_pointer
    int global_pointer_count = 13;
#endif

#ifdef start_gap
#define _extra_row
    int extra_row = 1;
    int row_swap_interval = 100;
#endif

#if defined(horizontal_wear_leveling) || defined(hwl)
    int group_bits = 64;
#endif

#ifdef column_wear_leveling
    int extra_col = 32;
    int col_swap_interval = 800;
    int ecp[8] = {32, 16, 8, 0};
#else
    int extra_col = 0;
#endif

#if defined(ilf) || defined(byte_rotation)
    int col_swap_interval = 256;
#endif

#ifdef retrofit
#define _extra_row
    int extra_row = 2;
    int retrofit_times_m = 100;
    int retrofit_times_n = 10;
#endif

#ifdef cai
    int row_swap_interval = 1024;
    int swap_counts = 8;
#endif

#ifndef _extra_row
    int extra_row = 0;
#endif

} cfg;

struct pos
{
    int col, row;
};

struct dataset {
    pos* data;
    int count;
};

#ifdef retrofit
struct gap {
    int pos;
    int times;

    gap(int pos) : gap(pos, 0) {}
    gap(int pos, int times) : pos(pos), times(times) {}

    bool operator==(const gap& other) const {
        return pos == other.pos;
    }
    
};

void move_gap_to(gap& g, int target, int* map) {
    //cout << "moving " << g.pos << " to " << target << endl;
    for (int i = 0; i < rows; ++i)
        if (map[i] == target)
        {
            map[i] = g.pos;
            g.pos = target;
            return;
        }
}

int find_next_gap(gap& g, vector<gap>& space, vector<gap>& error, vector<gap>& full, int totalheight)
{
    int target = g.pos;
    for (;;)
    {
        target = (target - 1 + totalheight) % totalheight;
        gap g = gap(target);
        if (std::any_of(space.begin(), space.end(), [target] (const gap& g) -> bool {return g.pos == target;})) continue;
        if (std::any_of(error.begin(), error.end(), [target] (const gap& g) -> bool {return g.pos == target;})) continue;
        if (std::any_of(full.begin(), full.end(), [target] (const gap& g) -> bool {return g.pos == target;})) continue;
        return target;
    }
}
#endif

pos rand_pos()
{
    pos p;
    p.col = cfg.col_rand->rand();
    p.row = cfg.row_rand->rand();
    return p;
}

dataset rand_cache(const char* filename, int count)
{
    /*
    auto* fp = fopen(filename, "r");
    if (fp)
    {
        auto* cache = new pos[count];
        int size = fread(cache, count * sizeof(pos), 1, fp);
        cout << "reading size = " << size << ", expecting = " << count * sizeof(pos) << endl;
        if (size == count * sizeof(pos))
        {
            auto res = dataset();
            res.data = cache;
            res.count = count;
            return res;
        }
        fclose(fp);
    }*/
    pos* cache = new pos[count];
    for (int i = 0; i < count; i++)
    {
        cache[i] = rand_pos();
    }
    /*
    fp = fopen(filename, "w");
    fwrite(cache, count * sizeof(pos), 1, fp);
    fclose(fp);*/
    
    auto res = dataset();
    res.data = cache;
    res.count = count;
    return res;
}

#ifdef ilf
int ilfmap[8][cols];
#endif

int total_crit_fault;

float **endurance = NULL;
int *fault_count = NULL;

int64_t emulate(float** endurance_t, dataset data)
{
    if (endurance)
    {
        for (int i = 0; i < rows + cfg.extra_row; ++i)
            delete[] endurance[i];
        delete[] endurance;
    }
    endurance = new float*[rows + cfg.extra_row];
    for (int i = 0; i < rows + cfg.extra_row; ++i)
        endurance[i] = new float[cols + cfg.extra_col];
    if (fault_count) delete[] fault_count;
    fault_count = new int[rows + cfg.extra_row];
    total_crit_fault = 0;
#ifdef cai
    int line_write_count[rows + cfg.extra_row];
    for (int i = 0; i < rows + cfg.extra_row; ++i)
        line_write_count[i] = 0;
    int row_swap_interval = cfg.row_swap_interval;
#endif

#if defined(horizontal_wear_leveling) || defined(hwl)
    int rotate = 0;
#endif

#ifdef rs_global_pointer
    bool stopwl = false;
#endif

#ifdef column_wear_leveling
    int page_min_fault = 0;
    int extra_col = cfg.ecp[page_min_fault];
    int col_swap_interval = cfg.col_swap_interval;
    int col_gap_pos = cols - 8;
    int target_gap = cols + extra_col - 8;
#else
    const int extra_col = 0;
#endif

#if defined(ilf) || defined(byte_rotation)
    int col_swap_interval[rows + cfg.extra_row];
    for (int i = 0; i < rows + cfg.extra_row; ++i) col_swap_interval[i] = cfg.col_swap_interval;
#endif

#ifdef ilf
    int ilf_status[rows + cfg.extra_row];
    for (int i = 0; i < rows + cfg.extra_row; ++i) ilf_status[i] = 0;
#endif

    for (int i = 0; i < rows + cfg.extra_row; i++)
    {
        for (int j = 0; j < cols + cfg.extra_col; j++)
        {
            endurance[i][j] = endurance_t[i][j];
#ifdef ecc
            if (j > cols) endurance[i][j] /= 256;
#endif
        }
        delete[] endurance_t[i];
    }
    delete[] endurance_t;

    memset(fault_count, 0, sizeof(int) * (rows + cfg.extra_row));

    int row_lut[rows];
    for (int i = 0; i < rows; ++i) row_lut[i] = i;
    int col_lut[cols];
    for (int i = 0; i < cols; ++i) col_lut[i] = i;

#ifdef row_sparing
    int row_sparing_count = 0;
#endif

#ifdef retrofit
    vector<gap> space, error, full;
    for (int i = 0; i < cfg.extra_row; ++i)
    {
        space.push_back(gap(i + rows, cfg.retrofit_times_m));
    }
#ifdef retrofit_global_pointer
    int line_count = 0;
#endif
#endif

#ifdef start_gap
    int gap = rows, next = rows - 1;
    int row_swap_interval = cfg.row_swap_interval;
#endif

    int64_t times = 0;
    rerun:
    for (int aaa = 0; aaa < data.count; ++aaa)
    {
        auto p = data.data[aaa];
        auto lp = p;
        ++times;

#ifdef uniform
        p.row = rand() % rows;
        p.col = rand() % cols;
#endif

#ifdef random
        int prev_row = p.row;
#endif
        p.row = row_lut[p.row];
        p.col = col_lut[p.col];

#ifdef cai
        ++line_write_count[p.row];
#endif

        auto prev_not_fault = (endurance[p.row][p.col] > 0);
        auto now_fault = --endurance[p.row][p.col] <= 0;

        if (cfg.crit_fault_every != 0 && times % cfg.crit_fault_every == 0)
        {
            endurance[p.row][p.col] = 0;
            ++total_crit_fault;
            now_fault = true;
        }

        if (prev_not_fault && now_fault)
        {
#ifdef cluster
            const int dir[8] = {0, 1, 0, -1, -1, 0, 1, 0};
            if (!reduced[p.row][p.col])
            {
                for (int i = 0; i < 4; ++i)
                {
                    int r = p.row + dir[i * 2];
                    int c = p.col + dir[i * 2 + 1];
                    if (r >= 0 && r < rows + cfg.extra_row && c >= 0 && c < cols + cfg.extra_col)
                    {
                        if (endurance[r][c] > cfg.mean / 10)
                        {
                            reduced[r][c] = true;
                            endurance[r][c] = cfg.mean / 10;
                        }
                    }
                }
            }
#endif
            ++fault_count[p.row];
#ifdef err_debug
            cout << "err = " << p.row << ", " << p.col << ", times = " << times << ", endurance = " << endurance[p.row][p.col] << ", fault = " << fault_count[p.row] << endl;
            cout << "fault map:" << endl;
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    cout << endurance[i][j] << " ";
                }
                cout << endl;
            }
#endif
            if (fault_count[p.row] > cfg.aux)
            {
#if defined(retrofit)
// pass
#elif defined(global_pointer)
                    if (page_total > cfg.global_pointer_count)
                        return times;
#elif defined(row_sparing)
//For RS, a row is replaced by a spare row when there are any unrecoverable faults that cannot be tolerated by the attached fault tolerance scheme. A page fails when a new row with unrecoverable faults emerges but there are no spare rows left.
                //if (fault_count[p.row] == cfg.aux + 1)
                {
#ifdef rs_global_pointer
                    if (row_sparing_count == cfg.rs_count) // no spare rows, we should judge the global pointer or the page will fail.
                    {
                        stopwl = true;
                        extra.push_back(times);
                        int page_total = 0;
                        int count = 0;
                        for (int i = 0; i < rows; ++i)
                        {
                            page_total += fault_count[row_lut[i]] < cfg.aux ? 0 : fault_count[row_lut[i]] - cfg.aux;
                        }
                        if (page_total > cfg.rs_count * 26)
                            return times;
                    }
                    else
                    {
                        ++row_sparing_count;
                        fault_count[p.row] = 0;
                        for (int i = 0; i < cols + extra_col; ++i)
                            endurance[p.row][i] = randnorm(cfg.mean, cfg.cov * cfg.mean);
                    }
#else
                    ++row_sparing_count;
                    if (row_sparing_count > cfg.rs_count)
                        return times;

                    fault_count[p.row] = 0;
                    for (int i = 0; i < cols + extra_col; ++i)
                        endurance[p.row][i] = randnorm(cfg.mean, cfg.cov * cfg.mean);
#endif
                }
#else
                return times;
#endif
            }

#ifdef column_wear_leveling
            if (fault_count[p.row] > page_min_fault)
            {
                page_min_fault = fault_count[p.row];
                for (int i = 0; i < cols; ++i) col_lut[i] = i;
                extra_col = cfg.ecp[page_min_fault];
                //cout << "extra = " << extra << ", times = " << times << endl;
            }
#endif

#ifdef retrofit
            if (!space.empty())
            {
                gap min = space[0];
                space.erase(space.begin());
                for (int i = 0; i < cols + extra_col; ++i)
                    endurance[p.row][i] -= cfg.swap_endurance;
                //cout << "moving " << min.pos << " to " << p.row << endl;
                //cout << "move empty to err ";
                move_gap_to(min, p.row, row_lut);
                error.push_back(min);
            }
            else
            {
                gap* min = NULL;
                for (auto& g : error)
                {
                    if (min == NULL || fault_count[min->pos] > fault_count[g.pos])
                    {
                        min = &g;
                    }
                }

                if (min != NULL && fault_count[min->pos] < fault_count[p.row])
                {
                    for (auto it = error.begin(); ;++it)
                        if (it->pos == min->pos)
                        {
                            auto t = *min;
                            min = &t;
                            error.erase(it);
                            break;
                        }
                    for (int i = 0; i < cols + extra_col; ++i)
                        endurance[p.row][i] -= cfg.swap_endurance;
                    //cout << "moving " << min->pos << " to " << p.row << endl;
                    //cout << "move to more err ";
                    move_gap_to(*min, p.row, row_lut);
                    (fault_count[min->pos] > cfg.aux ? full : error).push_back(*min);
                }
                else if (fault_count[p.row] > cfg.aux)
                {
// For RETROFIT_GPS, GPS activates once RETROFIT fails to protect a page. A page fails when a new unrecoverable fault emerges and the number of retired rows with PPPs equals the number of spare rows and all the PPPs are used up.
#ifdef retrofit_global_pointer
                    int page_total = 0;
                    int count = 0;
                    for (int i = 0; i < rows; ++i)
                    {
                        page_total += fault_count[row_lut[i]] < cfg.aux ? 0 : fault_count[row_lut[i]] - cfg.aux;
                        if (fault_count[row_lut[i]] > cfg.aux) ++count;
                    }
                    if (count > line_count)
                    {
                        line_count = count;
                        extra.push_back(times);
                        extra.push_back(count);
                    }
                    if (page_total > full.size() * 26)
                        return times;

#else
                    return times;
#endif
                }
            }
#endif
        }
// For ARS, the numbers of writes to each row are recorded by the counters. One quarter of the hottest rows are swapped with one quarter of the coldest rows every xx of writes.
#ifdef cai
        if (--row_swap_interval == 0)
        {
            row_swap_interval = cfg.row_swap_interval;

            int row_order[rows + cfg.extra_row];
            for (int i = 0; i < rows + cfg.extra_row; ++i) row_order[i] = i;
            std::sort(row_order, row_order + rows + cfg.extra_row, [&](int i1, int i2) -> bool { return line_write_count[i1] < line_write_count[i2]; });

            for (int i = 0; i < cfg.swap_counts; ++i)
            {
                int t = row_lut[row_order[i]];
                row_lut[row_order[i]] = row_lut[row_order[rows + cfg.extra_row - 1 - i]];
                row_lut[row_order[rows + cfg.extra_row - 1 - i]] = t;
                for (int j = 0; j < cols + extra_col; ++j)
                {
                    endurance[ row_lut[row_order[i]]][j] -= cfg.swap_endurance;
                    endurance[row_lut[row_order[rows + cfg.extra_row - 1 - i]]][j] -= cfg.swap_endurance;
                }
            }

            for (int i = 0; i < rows + cfg.extra_row; ++i) line_write_count[i] = 0;
        }
#endif

// For RETROFIT, spare rows are utilized to quarantine weak rows with more faults according to RETROFIT design. A page fails when the number of rows with unrecoverable faults is larger than the number of spare rows.
#ifdef retrofit
        for (auto& g : space)
        {
            if (--g.times <= 0)
            {
                int target = find_next_gap(g, space, error, full, rows + cfg.extra_row);
#ifdef hwl_plus
                float t = endurance[target][0];
                for (int i = 0; i < cols - 1; ++i)
                    endurance[target][i] = endurance[target][i + 1];
                endurance[target][cols - 1] = t;
#endif
#ifdef hwl
            float temp[cols];
            rotate = (rotate + 1) % cfg.group_bits;
            if (rotate == 0)
            {
                for (int i = 0; i < cols; ++i) temp[(i + 1) % cols] = endurance[target][i];
            }
            else
            {
                for (int i = 0; i < cols; ++i) temp[(i - cfg.group_bits + cols) % cols] = endurance[target][i];
            }
            memcpy(endurance[target], temp, sizeof(float) * cols);
#endif
                for (int i = 0; i < cols + extra_col; ++i)
                    endurance[target][i] -= cfg.swap_endurance;
                //cout << "auto move space ";
                move_gap_to(g, target, row_lut);
                g.times = cfg.retrofit_times_m * space.size();
            }
        }

        if (space.empty())
        {
            gap* min = NULL;
            for (auto& g : error)
            {
                if (min == NULL || fault_count[min->pos] > fault_count[g.pos]) min = &g;
            }

            if (min != NULL && --min->times <= 0)
            {
                int target = find_next_gap(*min, space, error, full, rows + cfg.extra_row);
                for (int i = 0; i < cols + extra_col; ++i)
                    endurance[target][i] -= cfg.swap_endurance;
                min->times = fault_count[min->pos] == 0 ? 
                    cfg.retrofit_times_n : 
                    cfg.retrofit_times_m * rows - cfg.retrofit_times_n * (rows - 1);
                for (int i = 0; i < cols + extra_col; ++i)
                    endurance[target][i] -= cfg.swap_endurance;
                //cout << "auto move err ";
// For the RETROFIT variants, whenever the moving gaps move to their neighbor rows, the neighbor rows are copied with the rows rotated by 1 position. 
#ifdef hwl_plus
                float t = endurance[target][0];
                for (int i = 0; i < cols - 1; ++i)
                    endurance[target][i] = endurance[target][i + 1];
                endurance[target][cols - 1] = t;
#endif
// For hwl, whenever the gap moves to its neighbor row, the neighbor row is copied with the row rotated by 1 position
#ifdef hwl
            float temp[cols];
            rotate = (rotate + 1) % cfg.group_bits;
            if (rotate == 0)
            {
                for (int i = 0; i < cols; ++i) temp[(i + 1) % cols] = endurance[target][i];
            }
            else
            {
                for (int i = 0; i < cols; ++i) temp[(i - cfg.group_bits + cols) % cols] = endurance[target][i];
            }
            memcpy(endurance[target], temp, sizeof(float) * cols);
#endif
                move_gap_to(*min, target, row_lut);
            }
        }
#endif

#ifdef column_wear_leveling
        if (--col_swap_interval == 0 && extra_col)
        {
            col_swap_interval = cfg.col_swap_interval;
            for (int j = 0; j < 8; ++j)
            {
                for (int i = 0; i < rows + cfg.extra_row; ++i)
                    endurance[i][target_gap + 7 - j] -= cfg.swap_endurance;
                //cout << "swapping " << col_lut[col_gap_pos + j] << " to " << target_gap + 7 - j << endl;
                col_lut[col_gap_pos + j] = target_gap + 7 - j;
            }
            target_gap = (target_gap - 8 + cols + extra_col) % (cols + extra_col);
            col_gap_pos = (col_gap_pos - 8 + cols) % cols;
        }
#endif

// For WoLFRaM, the accessed row has a probability of 1\% to be swapped with another randomly selected row.
#ifdef random
        if (rand() % (100 * 256 / 100)== 0)
        {
            int target = rand() % (rows + cfg.extra_row);
            for (int i = 0; i < cols + extra_col; ++i)
            {
                endurance[row_lut[target]][i] -= cfg.swap_endurance;
                endurance[row_lut[prev_row]][i] -= cfg.swap_endurance;
            }
            int t = row_lut[target];
            row_lut[target] = row_lut[prev_row];
            row_lut[prev_row] = t;
        }
#endif

// For SG, the writes to a page is balanced by the SG scheme, as illustrated in section~\ref{subsec:startgap}. The swapping is conducted every 100 writes. 

#ifdef start_gap
#ifdef rs_global_pointer
        if (!stopwl)
#endif
        if (--row_swap_interval == 0)
        {
            row_swap_interval = cfg.row_swap_interval;
            for (int i = 0; i < cols + extra_col; ++i) endurance[gap][i] -= cfg.swap_endurance;
#if defined(horizontal_wear_leveling)
            float temp[cols];
            rotate = (rotate + 1) % cfg.group_bits;
            if (rotate == 0)
            {
                for (int i = 0; i < cols; ++i) temp[(i + 1) % cols] = endurance[p.row][i];
            }
            else
            {
                for (int i = 0; i < cols; ++i) temp[(i - cfg.group_bits + cols) % cols] = endurance[p.row][i];
            }
            for (int i = 0; i < cols; ++i) endurance[p.row][i] = temp[i];
#elif defined(hwl_plus)
            float t = endurance[p.row][0];
            for (int i = 0; i < cols - 1; ++i)
                endurance[p.row][i] = endurance[p.row][i + 1];
            endurance[p.row][cols - 1] = t;
#endif
            row_lut[next] = gap;
            next = (next - 1 + rows) % rows;
            gap = (gap + rows) % (rows + 1);
        }
#endif

// For BLR, whenever the gap moves to its neighbor row, the neighbor row is copied with the row rotated by 1 byte.
#ifdef byte_rotation
        if (--col_swap_interval[p.row] == 0)
        {
            float temp[8];
            for (int i = 0; i < 8; ++i) temp[i] = endurance[p.row][i];
            for (int i = 0; i < cols - 8; ++i) endurance[p.row][i] = endurance[p.row][i + 8] - cfg.swap_endurance;
            for (int i = 0; i < 8; ++i) endurance[p.row][cols - 8 + i] = temp[i] - cfg.swap_endurance;
            col_swap_interval[p.row] = cfg.col_swap_interval;
        }
#endif

// For ILF, whenever the gap moves to its neighbor row, the neighbor row is copied with the row flipped according to enhanced ILF (each bit can be flipped to 8 positions)
#ifdef ilf
        if (--col_swap_interval[p.row] == 0)
        {
            float temp[cols];
            for (int i = 0; i < cols; ++i) temp[i] = endurance[p.row][ilfmap[ilf_status[p.row]][i]];
            ilf_status[p.row] = (ilf_status[p.row] + 1) & 7;
            for (int i = 0; i < cols; ++i) endurance[p.row][i] = temp[ilfmap[ilf_status[p.row]][i]] - cfg.swap_endurance;
            col_swap_interval[p.row] = cfg.col_swap_interval;
        }
#endif
    }

    goto rerun;
}

float** rand_matrix(int mean, int std)
{
    //cout << "random " << (rows + cfg.extra_row) << "x" << (cols + cfg.extra_col) << " matrix" << endl;
    float** matrix = new float*[rows + cfg.extra_row];
    for (int i = 0; i < rows + cfg.extra_row; i++)
    {
        matrix[i] = new float[cols + cfg.extra_col];
        for (int j = 0; j < cols + cfg.extra_col; j++)
        {
            matrix[i][j] = randnorm(mean, std);
            if (matrix[i][j] <= 0) matrix[i][j] = mean;
        }
    }

    for (int k = 0; k < cfg.start_fault; ++k)
    {
        int x = rand() % (rows + cfg.extra_row);
        int y = rand() % (cols + cfg.extra_col);
        matrix[x][y] = 1e-6;
    }
    return matrix;
}

void init();

void _init()
{

#ifdef ecp4
    ::cfg.aux = 4;
#ifdef column_wear_leveling
    ::cfg.extra_col = 40;
    ::cfg.ecp[0] = 40;
    ::cfg.ecp[1] = 32;
    ::cfg.ecp[2] = 16;
    ::cfg.ecp[3] = 8;
    ::cfg.ecp[4] = 0;
#endif
#endif

#ifdef ecp3
    ::cfg.aux = 3;
#ifdef column_wear_leveling
    ::cfg.extra_col = 32;
    ::cfg.ecp[0] = 32;
    ::cfg.ecp[1] = 16;
    ::cfg.ecp[2] = 8;
    ::cfg.ecp[3] = 0;
#endif
#endif

#ifdef ecp2
    ::cfg.aux = 2;
#ifdef column_wear_leveling
    ::cfg.extra_col = 16;
    ::cfg.ecp[0] = 16;
    ::cfg.ecp[1] = 8;
    ::cfg.ecp[2] = 0;
#endif
#endif

#ifdef ecp1
    ::cfg.aux = 1;
#ifdef column_wear_leveling
    ::cfg.extra_col = 8;
    ::cfg.ecp[0] = 8;
    ::cfg.ecp[1] = 0;
#endif
#endif

#ifdef ecp0
    ::cfg.aux = 0;
#endif

#ifdef hwl_plus
    ::cfg.mean = ::cfg.mean * (ecp_bit_num[::cfg.aux] + cols) / cols;
#endif

#ifdef ecc
    ::cfg.extra_col = 9;
#endif
}

int main(int argc, char *argv[])
{
    _init();
    init();

#ifdef ilf
    for (int i = 0; i < cols; ++i) ilfmap[0][i] = i;
    for (int i = 1; i < 8; ++i)
        for (int j = 0; j < cols; ++j)
            ilfmap[i][j] = ilfmap[0][(j >> (2 + i) << (2 + i)) | (((1 << (2 + i)) - 1) & ~j)];
#endif
    auto dataset = rand_cache("cache.bin", 1 << 26);
    if (argc == 2)
        freopen(argv[1], "a", stdout);
    if (cfg.savefile != NULL)
        freopen(cfg.savefile, "a", stdout);
    int64_t s = 0, n = 0;
    for (int i = 0; i < 200; ++i)
    {
        srand(0x114514 * i + 0x1919810);
        int64_t x = emulate(rand_matrix(cfg.mean, (int)(cfg.mean * cfg.cov)), dataset);
        s += x;
        ++n;

        int y = 0;
        for (int i = 0; i < rows + cfg.extra_row; ++i)
            y += min(fault_count[i], cfg.aux);
        std::cout << x << " " << ((float)y / (rows + cfg.extra_row) / cfg.aux);
        if (extra.size() != 0)
        {
            std::cout << " ";
            for (auto& x : extra)
                std::cout << x << " ";
            extra.clear();
        }
        std::cout << " " << s / n << endl;
    }

    return 0;
}