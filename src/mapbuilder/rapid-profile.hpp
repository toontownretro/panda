#ifndef RAPID_PROFILE__HPP_
#define RAPID_PROFILE__HPP_

//----------------------------------------------------------------------------//
// INCLUDES
//----------------------------------------------------------------------------//

#include <cassert>
#include <chrono>
#include <csignal>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <vector>

//----------------------------------------------------------------------------//
// SETTINGS
//----------------------------------------------------------------------------//

// Define to disable rapid profile
// #define RAPID_PROFILE_DSIABLE

// Maximum size of the name and file strings
#define RAPID_PROFILE_STR_SIZE 64

// Maximum number of interval timers (including internal)
#define RAPID_PROFILE_MAX_TIMERS 1024

// Number of interval allocated per chunk
#define RAPID_PROFILE_CHUNK_SIZE 1048576

// Thread safety
#define RAPID_PROFILE_THREAD_SAFE 1

// Internal timers
#define RAPID_PROFILE_INTERNAL 1

//----------------------------------------------------------------------------//
// NOW
//----------------------------------------------------------------------------//

#ifdef RAPID_PROFILE_DISABLE
#define RAPID_PROFILE_NOW()
#else
#define RAPID_PROFILE_NOW() RapidProfile::type::clock::now()
#endif

//----------------------------------------------------------------------------//
// INIT
//----------------------------------------------------------------------------//

#ifdef RAPID_PROFILE_DISABLE
#define RAPID_PROFILE_INIT()
#else
#define RAPID_PROFILE_INIT() RapidProfile::api<RAPID_PROFILE_STR_SIZE>::init()
#endif

//----------------------------------------------------------------------------//
// INTERVAL
//----------------------------------------------------------------------------//

#ifdef RAPID_PROFILE_DISABLE
#define RAPID_PROFILE_INTERVAL_ID(NAME)
#define RAPID_PROFILE_INTERVAL_INST(NAME)
#else
#define RAPID_PROFILE_INTERVAL_ID(NAME) _rapid_profile_interval_##NAME##_id
#define RAPID_PROFILE_INTERVAL_INST(NAME) _rapid_profile_interval_##NAME
#endif

//----------------------------------------------------------------------------//

#ifdef RAPID_PROFILE_DISABLE
#define INTERVAL(NAME)
#else
#define INTERVAL(NAME)                                                                \
    static RapidProfile::type::id RAPID_PROFILE_INTERVAL_ID(NAME) =                   \
        RapidProfile::api<RAPID_PROFILE_STR_SIZE>::get_id(#NAME, __FILE__, __LINE__); \
    RapidProfile::interval & RAPID_PROFILE_INTERVAL_INST(NAME) =                      \
        RapidProfile::api<RAPID_PROFILE_STR_SIZE>::get_interval();                    \
    RAPID_PROFILE_INTERVAL_INST(NAME).id    = RAPID_PROFILE_INTERVAL_ID(NAME);        \
    RAPID_PROFILE_INTERVAL_INST(NAME).start = RAPID_PROFILE_NOW();
#endif

//----------------------------------------------------------------------------//

#ifdef RAPID_PROFILE_DISABLE
#define INTERVAL_END(NAME)
#else
#define INTERVAL_END(NAME) RAPID_PROFILE_INTERVAL_INST(NAME).stop = RAPID_PROFILE_NOW();
#endif

//----------------------------------------------------------------------------//

#ifdef RAPID_PROFILE_DISABLE
#define INTERVAL_START(NAME)
#else
#define INTERVAL_START(NAME) RAPID_PROFILE_INTERVAL_INST(NAME).start = RAPID_PROFILE_NOW();
#endif

//----------------------------------------------------------------------------//
// THREAD SAFETY
//----------------------------------------------------------------------------//

#if RAPID_PROFILE_THREAD_SAFE == 1

#define RAPID_PROFILE_MUTEX(NAME) \
    static std::mutex & NAME()    \
    {                             \
        static std::mutex NAME;   \
        return NAME;              \
    }

#define RAPID_PROFILE_MUTEX_GUARD(NAME) std::lock_guard<std::mutex> guard(NAME());

#else

#define RAPID_PROFILE_MUTEX(NAME)
#define RAPID_PROFILE_MUTEX_GUARD(NAME)

#endif

//----------------------------------------------------------------------------//
// INTERNAL
//----------------------------------------------------------------------------//

#define RAPID_PROFILE_INIT_ID 0
#define RAPID_PROFILE_RECHUNK_ID 1

//----------------------------------------------------------------------------//

namespace RapidProfile
{
//----------------------------------------------------------------------------//
// Types
//----------------------------------------------------------------------------//

namespace type
{
typedef unsigned int                id;
typedef float                       time;
typedef std::chrono::steady_clock   clock;
typedef std::chrono::duration<time> duration;
typedef clock::time_point           time_point;

}  // namespace type

//----------------------------------------------------------------------------//
// Interval
//----------------------------------------------------------------------------//

struct interval
{
    interval()
    {
    }
    interval(const type::id id) : id(id), start(RAPID_PROFILE_NOW())
    {
    }
    type::time duration() const
    {
        return type::duration(stop - start).count();
    }
    type::id         id;
    type::time_point start;
    type::time_point stop;

    template <size_t N>
    struct tag
    {
        char name[N];
        char file[N];
        int  line;
    };
};

//----------------------------------------------------------------------------//
// Chunker
//----------------------------------------------------------------------------//

template <class T>
class chunker
{
  public:
    //------------------------------------------------------------------------//

    chunker(size_t chunk_size) : chunk_size_(chunk_size), chunk_(NULL)
    {
        chunk_ = new std::vector<T>();
        chunk_->reserve(chunk_size_);
        chunks_.push_back(chunk_);

        rechunk();
    }

    //------------------------------------------------------------------------//

    ~chunker()
    {
        for (typename std::list<std::vector<T> *>::iterator it = chunks_.begin(); it != chunks_.end(); it++) delete *it;
        chunks_.clear();
    }

    //------------------------------------------------------------------------//

    T & push_back(T const & item = T())
    {
        if (chunk_->size() + 1 == chunk_size_) rechunk();

        chunk_->push_back(item);
        return chunk_->back();
    }

    //------------------------------------------------------------------------//

    std::list<std::vector<T> *> & chunks()
    {
        return chunks_;
    }

    //------------------------------------------------------------------------//

  protected:
    //------------------------------------------------------------------------//

    void rechunk()
    {
#if RAPID_PROFILE_INTERNAL == 1
        interval rechunk_interval(RAPID_PROFILE_RECHUNK_ID);
#endif

        std::vector<T> * prev_chunk = chunk_;

        chunk_ = chunks_.back();

        std::vector<T> * next = new std::vector<T>();
        next->reserve(chunk_size_);
        chunks_.push_back(next);

#if RAPID_PROFILE_INTERNAL == 1
        rechunk_interval.stop = RAPID_PROFILE_NOW();
        prev_chunk->push_back(rechunk_interval);
#endif
    }

    //------------------------------------------------------------------------//

  private:
    size_t                      chunk_size_;
    std::vector<T> *            chunk_;
    std::list<std::vector<T> *> chunks_;
};

//----------------------------------------------------------------------------//
// API
//----------------------------------------------------------------------------//

template <size_t N>
class api
{
  public:
    //------------------------------------------------------------------------//

    static void init()
    {
#if RAPID_PROFILE_INTERNAL == 1
        RapidProfile::type::time_point const & start = start_time();
#else
        start_time();
#endif

        tags().reserve(RAPID_PROFILE_MAX_TIMERS);

#if RAPID_PROFILE_INTERNAL == 1
        assert(RAPID_PROFILE_INIT_ID == get_id("RAPID_PROFILE_INIT", __FILE__, __LINE__));
        assert(RAPID_PROFILE_RECHUNK_ID == get_id("RAPID_PROFILE_RECHUNK", __FILE__, __LINE__));

        interval & startup_interval = get_interval();
        startup_interval.id         = RAPID_PROFILE_INIT_ID;
        startup_interval.start      = start;
#endif

        std::atexit(api::exit);
        ::signal(SIGINT, api::signal);

#if RAPID_PROFILE_INTERNAL == 1
        startup_interval.stop = RAPID_PROFILE_NOW();
#endif
    }

    //------------------------------------------------------------------------//

    static interval & get_interval()
    {
        RAPID_PROFILE_MUTEX_GUARD(interval_mutex);

        return intervals().push_back();
    }

    //------------------------------------------------------------------------//

    static type::id get_id(const char * name, const char * file, int line)
    {
        static type::id id = 0;

        RAPID_PROFILE_MUTEX_GUARD(tag_mutex);

        tags().push_back(interval::tag<N>());
        interval::tag<N> & tag = tags().back();

        strncpy(tag.name, name, N);
        tag.name[N - 1] = '\0';

        strncpy(tag.file, file, N);
        tag.file[N - 1] = '\0';

        tag.line = line;

        return id++;
    }

    //------------------------------------------------------------------------//

  private:
    //------------------------------------------------------------------------//

    static void exit()
    {
        log();
    }

    //------------------------------------------------------------------------//

    static void signal(int signum)
    {
        ::exit(signum);
    }

    //------------------------------------------------------------------------//

    static type::time relative(type::time_point const & stop  = type::clock::now(),
                               type::time_point const & start = start_time())
    {
        return type::duration(stop - start).count();
    }

    //------------------------------------------------------------------------//

    static void log()
    {
        std::ofstream file;

        std::vector<interval::tag<N> > &     tags   = api::tags();
        std::list<std::vector<interval> *> & chunks = intervals().chunks();

        file.open("tags.rp.csv");
        file << "id,name,file,line" << std::endl;
        for (typename std::vector<interval::tag<N> >::iterator it = tags.begin(); it != tags.end(); it++)
        {
            file << (it - tags.begin()) << ',' << it->name << ',' << it->file << ',' << it->line << std::endl;
        }
        file.close();

        file.open("intervals.rp.csv");
        file << "id,start,stop,duration" << std::endl;
        for (std::list<std::vector<interval> *>::iterator it = chunks.begin(); it != chunks.end(); it++)
        {
            for (std::vector<interval>::iterator vit = (*it)->begin(); vit != (*it)->end(); vit++)
            {
                file << vit->id << ',' << relative(vit->start) * 1e6 << ',' << relative(vit->stop) * 1e6 << ','
                     << relative(vit->stop, vit->start) * 1e6 << std::endl;
            }
        }
        file.close();
    }

  private:
    //------------------------------------------------------------------------//

    RAPID_PROFILE_MUTEX(interval_mutex);
    RAPID_PROFILE_MUTEX(tag_mutex);

    //------------------------------------------------------------------------//

    static type::time_point const & start_time()
    {
        static type::time_point start_time = RAPID_PROFILE_NOW();
        return start_time;
    }

    //------------------------------------------------------------------------//

    static std::vector<interval::tag<N> > & tags()
    {
        static std::vector<interval::tag<N> > tags;
        return tags;
    }

    //------------------------------------------------------------------------//

    static chunker<interval> & intervals()
    {
        static chunker<interval> * intervals = new chunker<interval>(RAPID_PROFILE_CHUNK_SIZE);
        return *intervals;
    }

    //------------------------------------------------------------------------//
};

//----------------------------------------------------------------------------//

};  // namespace RapidProfile

#endif
