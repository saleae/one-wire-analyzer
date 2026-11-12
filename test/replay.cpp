// Replays a Logic 2 raw edge export ("Time [s],bus") through the real OneWireAnalyzer::WorkerThread() and prints
// one line per transaction. See README.md in this directory.
//   replay <raw.csv> [--std-ns N] [--ovd-ns N] [--rate HZ]
#include "mock_sdk.h"
#include "OneWireAnalyzer.h"
#include "OneWireAnalyzerSettings.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

struct TestAnalyzer : OneWireAnalyzer
{
    void Configure( int std_ns, int ovd_ns )
    {
#ifndef BASELINE // BASELINE builds the pristine upstream sources, which have no sample point settings
        if( std_ns >= 0 ) mSettings->mStdSamplePointNs = std_ns;
        if( ovd_ns >= 0 ) mSettings->mOvdSamplePointNs = ovd_ns;
#endif
        mSettings->mOneWireChannel = Channel( 1, 0, DIGITAL_CHANNEL );
    }
};

static bool LoadCsv( const char* path, double rate )
{
    std::ifstream f( path );
    if( !f ) return false;
    std::string line;
    std::getline( f, line );
    int prev = -1;
    double t0 = 0;
    bool have_t0 = false;
    while( std::getline( f, line ) )
    {
        double t; int v;
        if( sscanf( line.c_str(), "%lf,%d", &t, &v ) != 2 ) continue;
        if( v == prev ) continue; // only transitions
        if( prev < 0 )
        {
            if( v != 1 ) { fprintf( stderr, "capture must idle high\n" ); return false; }
            prev = v;
            continue;
        }
        if( !have_t0 ) { t0 = t; have_t0 = true; }
        gCapture.edges.push_back( (unsigned long long)llround( ( t - t0 ) * rate ) );
        prev = v;
    }
    return !gCapture.edges.empty();
}

int main( int argc, char** argv )
{
    if( argc < 2 ) { fprintf( stderr, "usage: replay raw.csv [--std-ns N] [--ovd-ns N] [--rate HZ]\n" ); return 2; }
    int std_ns = -1, ovd_ns = -1;
    double rate = 12500000;
    for( int i = 2; i + 1 < argc; i += 2 )
    {
        if( !strcmp( argv[ i ], "--std-ns" ) ) std_ns = atoi( argv[ i + 1 ] );
        else if( !strcmp( argv[ i ], "--ovd-ns" ) ) ovd_ns = atoi( argv[ i + 1 ] );
        else if( !strcmp( argv[ i ], "--rate" ) ) rate = atof( argv[ i + 1 ] );
    }
    gCapture.sample_rate = (unsigned)rate;
    if( !LoadCsv( argv[ 1 ], rate ) ) return 2;

    TestAnalyzer a;
    a.Configure( std_ns, ovd_ns );
    a.SetupResults();
    try { a.WorkerThread(); } catch( MockEndOfData& ) {}

    // group frames into transactions (a reset frame opens one)
    std::ostringstream out;
    int txn = 0;
    std::string line;
    auto flush = [&] { if( !line.empty() ) out << line << "\n"; line.clear(); };
    for( auto& fr : gFrames )
    {
        auto get = [&]( const char* k ) { for( auto& kv : fr.kv ) if( kv.first == k ) return kv.second; return std::string(); };
        if( fr.type == "reset" ) { flush(); char b[ 64 ]; snprintf( b, sizeof b, "%03d t=%.6f reset", ++txn, fr.start / rate ); line = b; }
        else if( fr.type == "presence" ) line += " presence";
        else if( fr.type == "rom_command" ) line += " cmd=" + get( "description" ) + ":" + get( "rom_command" );
        else if( fr.type == "alarm" || fr.type == "invalid_rom_command" ) line += " cmd=" + fr.type + ":" + get( "rom_command" );
        else if( fr.type == "family_code" ) line += " fam=" + get( "family" );
        else if( fr.type == "id" ) line += " id=" + get( "id" );
        else if( fr.type == "crc" ) line += " crc=" + get( "crc" );
        else if( fr.type == "data" ) { char b[ 8 ]; snprintf( b, sizeof b, "%02x", (unsigned)strtoul( get( "data" ).c_str(), 0, 16 ) ); line += " "; line += b; }
    }
    flush();
    fputs( out.str().c_str(), stdout );
    return 0;
}
