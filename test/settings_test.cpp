// Settings persistence: new fields round-trip, and a settings string saved by the pre-fix analyzer still loads.
#include "OneWireAnalyzer.h"
#include "OneWireAnalyzerSettings.h"
#include <cstdio>
#include <string>
#define CHECK( c ) do { if( !( c ) ) { printf( "FAIL %s:%d %s\n", __FILE__, __LINE__, #c ); return 1; } } while( 0 )
int main()
{
    OneWireAnalyzerSettings a;
    CHECK( a.mStdSamplePointNs == 13000 && a.mOvdSamplePointNs == 1900 ); // defaults are correct unconfigured
    a.mStdSamplePointNs = 12000; a.mOvdSamplePointNs = 1700;
    std::string saved = a.SaveSettings();
    OneWireAnalyzerSettings b;
    b.LoadSettings( saved.c_str() );
    CHECK( b.mStdSamplePointNs == 12000 && b.mOvdSamplePointNs == 1700 );
    // pre-fix format: name, channel (device index type), overdrive flag only
    OneWireAnalyzerSettings c;
    c.LoadSettings( "SaleaeOneWireAnalyzer 1 0 1 0 " );
    CHECK( c.mStdSamplePointNs == 13000 && c.mOvdSamplePointNs == 1900 && !c.mOverdrive );
    printf( "settings ok\n" );
    return 0;
}
