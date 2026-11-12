// Minimal host-side stand-in for the Saleae Analyzer SDK runtime (libAnalyzer), just enough to run the real
// OneWireAnalyzer::WorkerThread() over an edge list on the command line, without Logic 2.
// The SDK headers are the real ones; only the out-of-line members that the analyzer touches are implemented here.
#include "mock_sdk.h"

#include <Analyzer.h>
#include <AnalyzerChannelData.h>
#include <AnalyzerHelpers.h>
#include <AnalyzerResults.h>
#include <AnalyzerSettings.h>
#include <AnalyzerSettingInterface.h>
#include <SimulationChannelDescriptor.h>

#include "OneWireAnalyzer.h"

#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>

// ---- replay state -------------------------------------------------------------------------------------------
MockCapture gCapture;
std::vector<MockFrame> gFrames;
static std::vector<std::pair<std::string, std::string>> gCurrentV2;

struct AnalyzerChannelDataData
{
    size_t next = 0; // index of next edge in gCapture.edges
    U64 pos = 0;
    BitState state = BIT_HIGH;
};

static U64 NextEdgeOrThrow( AnalyzerChannelDataData* d )
{
    if( d->next >= gCapture.edges.size() )
        throw MockEndOfData();
    return gCapture.edges[ d->next ];
}

AnalyzerChannelData::AnalyzerChannelData( ChannelData* ) : mData( new AnalyzerChannelDataData ) {}
AnalyzerChannelData::~AnalyzerChannelData() { delete mData; }
U64 AnalyzerChannelData::GetSampleNumber() { return mData->pos; }
BitState AnalyzerChannelData::GetBitState() { return mData->state; }
U64 AnalyzerChannelData::GetSampleOfNextEdge() { return NextEdgeOrThrow( mData ); }
void AnalyzerChannelData::AdvanceToNextEdge()
{
    mData->pos = NextEdgeOrThrow( mData );
    mData->next++;
    mData->state = Invert( mData->state );
}
U32 AnalyzerChannelData::Advance( U32 n )
{
    return AdvanceToAbsPosition( mData->pos + n );
}
U32 AnalyzerChannelData::AdvanceToAbsPosition( U64 target )
{
    U32 changes = 0;
    while( mData->next < gCapture.edges.size() && gCapture.edges[ mData->next ] <= target )
    {
        mData->next++;
        mData->state = Invert( mData->state );
        changes++;
    }
    if( mData->next >= gCapture.edges.size() && target > gCapture.edges.back() + 1000000 )
        throw MockEndOfData();
    mData->pos = target;
    return changes;
}
bool AnalyzerChannelData::WouldAdvancingCauseTransition( U32 n ) { return mData->next < gCapture.edges.size() && gCapture.edges[ mData->next ] <= mData->pos + n; }
bool AnalyzerChannelData::WouldAdvancingToAbsPositionCauseTransition( U64 t ) { return mData->next < gCapture.edges.size() && gCapture.edges[ mData->next ] <= t; }
void AnalyzerChannelData::TrackMinimumPulseWidth() {}
U64 AnalyzerChannelData::GetMinimumPulseWidthSoFar() { return 0; }
bool AnalyzerChannelData::DoMoreTransitionsExistInCurrentData() { return mData->next < gCapture.edges.size(); }

// ---- Analyzer -----------------------------------------------------------------------------------------------
struct AnalyzerData
{
    AnalyzerChannelData* chan = nullptr;
    AnalyzerResults* results = nullptr;
    AnalyzerSettings* settings = nullptr;
};
Analyzer::Analyzer() : mData( new AnalyzerData ) {}
Analyzer::~Analyzer() { delete mData; }
Analyzer2::Analyzer2() {}
void Analyzer2::SetupResults() {}
void Analyzer::SetAnalyzerSettings( AnalyzerSettings* s ) { mData->settings = s; }
void Analyzer::KillThread() {}
AnalyzerChannelData* Analyzer::GetAnalyzerChannelData( Channel& )
{
    if( !mData->chan )
    {
        mData->chan = new AnalyzerChannelData( nullptr );
        mData->chan->AdvanceToAbsPosition( 0 );
    }
    return mData->chan;
}
void Analyzer::ReportProgress( U64 ) {}
void Analyzer::SetAnalyzerResults( AnalyzerResults* r ) { mData->results = r; }
U32 Analyzer::GetSimulationSampleRate() { return gCapture.sample_rate; }
U32 Analyzer::GetSampleRate() { return gCapture.sample_rate; }
void Analyzer::UseFrameV2() {}
void Analyzer::CheckIfThreadShouldExit() {}

// ---- Results ------------------------------------------------------------------------------------------------
class FrameV2Data
{
  public:
    std::vector<std::pair<std::string, std::string>> kv;
};
FrameV2::FrameV2() : mInternals( new FrameV2Data ) {}
FrameV2::~FrameV2() { delete mInternals; }
static std::string Hex( unsigned long long v ) { char b[ 32 ]; snprintf( b, sizeof b, "%llx", v ); return b; }
void FrameV2::AddString( const char* k, const char* v ) { mInternals->kv.push_back( { k, v } ); }
void FrameV2::AddDouble( const char* k, double v ) { mInternals->kv.push_back( { k, std::to_string( v ) } ); }
void FrameV2::AddInteger( const char* k, S64 v ) { mInternals->kv.push_back( { k, Hex( v ) } ); }
void FrameV2::AddBoolean( const char* k, bool v ) { mInternals->kv.push_back( { k, v ? "1" : "0" } ); }
void FrameV2::AddByte( const char* k, U8 v ) { mInternals->kv.push_back( { k, Hex( v ) } ); }
void FrameV2::AddByteArray( const char* k, const U8*, U64 ) { mInternals->kv.push_back( { k, "" } ); }

Frame::Frame() : mStartingSampleInclusive( 0 ), mEndingSampleInclusive( 0 ), mData1( 0 ), mData2( 0 ), mType( 0 ), mFlags( 0 ) {}
Frame::Frame( const Frame& f ) = default;
Frame::~Frame() {}
bool Frame::HasFlag( U8 flag ) { return ( mFlags & flag ) != 0; }

struct AnalyzerResultsData {};
AnalyzerResults::AnalyzerResults() : mData( nullptr ) {}
AnalyzerResults::~AnalyzerResults() {}
void AnalyzerResults::AddFrameV2( const FrameV2& f, const char* type, U64 s, U64 e )
{
    MockFrame mf;
    mf.type = type;
    mf.start = s;
    mf.end = e;
    mf.kv = f.mInternals->kv;
    gFrames.push_back( mf );
}
U64 AnalyzerResults::AddFrame( const Frame& ) { return 0; }
U64 AnalyzerResults::CommitPacketAndStartNewPacket() { return 0; }
void AnalyzerResults::AddChannelBubblesWillAppearOn( const Channel& ) {}
void AnalyzerResults::CommitResults() {}

OneWireAnalyzerResults::OneWireAnalyzerResults( OneWireAnalyzer* a, OneWireAnalyzerSettings* s ) : AnalyzerResults(), mSettings( s ), mAnalyzer( a ) {}
OneWireAnalyzerResults::~OneWireAnalyzerResults() {}
void OneWireAnalyzerResults::GenerateBubbleText( U64, Channel&, DisplayBase ) {}
void OneWireAnalyzerResults::GenerateExportFile( const char*, DisplayBase, U32 ) {}
void OneWireAnalyzerResults::GenerateFrameTabularText( U64, DisplayBase ) {}
void OneWireAnalyzerResults::GeneratePacketTabularText( U64, DisplayBase ) {}
void OneWireAnalyzerResults::GenerateTransactionTabularText( U64, DisplayBase ) {}

// simulation generator is not exercised here
OneWireSimulationDataGenerator::OneWireSimulationDataGenerator() {}
OneWireSimulationDataGenerator::~OneWireSimulationDataGenerator() {}
void OneWireSimulationDataGenerator::Initialize( U32, OneWireAnalyzerSettings* ) {}
U32 OneWireSimulationDataGenerator::GenerateSimulationData( U64, U32, SimulationChannelDescriptor** ) { return 0; }
SimulationChannelDescriptor::SimulationChannelDescriptor() : mData( nullptr ) {}
SimulationChannelDescriptor::~SimulationChannelDescriptor() {}

// ---- Settings + interfaces -----------------------------------------------------------------------------------
Channel::Channel() : mDeviceId( 0 ), mChannelIndex( 0 ), mDataType( DIGITAL_CHANNEL ) {}
Channel::Channel( const Channel& c ) = default;
Channel::Channel( U64 d, U32 i, ChannelDataType t ) : mDeviceId( d ), mChannelIndex( i ), mDataType( t ) {}
Channel::~Channel() {}
Channel& Channel::operator=( const Channel& ) = default;

struct AnalyzerSettingInterfaceData { std::string title, tip; };
AnalyzerSettingInterface::AnalyzerSettingInterface() : mData( new AnalyzerSettingInterfaceData ) {}
AnalyzerSettingInterface::~AnalyzerSettingInterface() { delete mData; }
void AnalyzerSettingInterface::operator delete( void* p ) { ::operator delete( p ); }
void* AnalyzerSettingInterface::operator new( size_t n ) { return ::operator new( n ); }
AnalyzerInterfaceTypeId AnalyzerSettingInterface::GetType() { return INTERFACE_BASE; }
void AnalyzerSettingInterface::SetTitleAndTooltip( const char* t, const char* tip ) { mData->title = t; mData->tip = tip; }
const char* AnalyzerSettingInterface::GetTitle() { return mData->title.c_str(); }

struct AnalyzerSettingInterfaceChannelData { Channel c; };
AnalyzerSettingInterfaceChannel::AnalyzerSettingInterfaceChannel() : mChannelData( new AnalyzerSettingInterfaceChannelData ) {}
AnalyzerSettingInterfaceChannel::~AnalyzerSettingInterfaceChannel() { delete mChannelData; }
AnalyzerInterfaceTypeId AnalyzerSettingInterfaceChannel::GetType() { return INTERFACE_CHANNEL; }
Channel AnalyzerSettingInterfaceChannel::GetChannel() { return mChannelData->c; }
void AnalyzerSettingInterfaceChannel::SetChannel( const Channel& c ) { mChannelData->c = c; }

struct AnalyzerSettingInterfaceBoolData { bool v = false; };
AnalyzerSettingInterfaceBool::AnalyzerSettingInterfaceBool() : mBoolData( new AnalyzerSettingInterfaceBoolData ) {}
AnalyzerSettingInterfaceBool::~AnalyzerSettingInterfaceBool() { delete mBoolData; }
AnalyzerInterfaceTypeId AnalyzerSettingInterfaceBool::GetType() { return INTERFACE_BOOL; }
bool AnalyzerSettingInterfaceBool::GetValue() { return mBoolData->v; }
void AnalyzerSettingInterfaceBool::SetValue( bool v ) { mBoolData->v = v; }
void AnalyzerSettingInterfaceBool::SetCheckBoxText( const char* ) {}

struct AnalyzerSettingInterfaceIntegerData { int v = 0, min = 0, max = 0; };
AnalyzerSettingInterfaceInteger::AnalyzerSettingInterfaceInteger() : mIntegerData( new AnalyzerSettingInterfaceIntegerData ) {}
AnalyzerSettingInterfaceInteger::~AnalyzerSettingInterfaceInteger() { delete mIntegerData; }
AnalyzerInterfaceTypeId AnalyzerSettingInterfaceInteger::GetType() { return INTERFACE_INTEGER; }
int AnalyzerSettingInterfaceInteger::GetInteger() { return mIntegerData->v; }
void AnalyzerSettingInterfaceInteger::SetInteger( int v ) { mIntegerData->v = v; }
void AnalyzerSettingInterfaceInteger::SetMin( int v ) { mIntegerData->min = v; }
void AnalyzerSettingInterfaceInteger::SetMax( int v ) { mIntegerData->max = v; }

struct AnalyzerSettingsData { std::string ret; };
AnalyzerSettings::AnalyzerSettings() : mData( new AnalyzerSettingsData ) {}
AnalyzerSettings::~AnalyzerSettings() { delete mData; }
void AnalyzerSettings::ClearChannels() {}
void AnalyzerSettings::AddChannel( Channel&, const char*, bool ) {}
void AnalyzerSettings::AddInterface( AnalyzerSettingInterface* ) {}
void AnalyzerSettings::AddExportOption( U32, const char* ) {}
void AnalyzerSettings::AddExportExtension( U32, const char*, const char* ) {}
const char* AnalyzerSettings::SetReturnString( const char* s ) { mData->ret = s; return mData->ret.c_str(); }

void AnalyzerHelpers::Assert( const char* m ) { fprintf( stderr, "ASSERT: %s\n", m ); abort(); }

// whitespace-separated token archive (the real one is similar in spirit; only round-trip fidelity matters here)
struct SimpleArchiveData { std::vector<std::string> toks; size_t rd = 0; std::string out; };
SimpleArchive::SimpleArchive() : mData( new SimpleArchiveData ) {}
SimpleArchive::~SimpleArchive() { delete mData; }
void SimpleArchive::SetString( const char* s )
{
    mData->toks.clear(); mData->rd = 0;
    std::string cur;
    for( const char* p = s; ; ++p )
    {
        if( *p == ' ' || *p == 0 ) { if( !cur.empty() ) mData->toks.push_back( cur ); cur.clear(); if( !*p ) break; }
        else cur += *p;
    }
}
const char* SimpleArchive::GetString() { return mData->out.c_str(); }
static bool Put( SimpleArchiveData* d, const std::string& s ) { d->out += s + " "; return true; }
bool SimpleArchive::operator<<( U64 v ) { return Put( mData, std::to_string( v ) ); }
bool SimpleArchive::operator<<( U32 v ) { return Put( mData, std::to_string( v ) ); }
bool SimpleArchive::operator<<( S64 v ) { return Put( mData, std::to_string( v ) ); }
bool SimpleArchive::operator<<( S32 v ) { return Put( mData, std::to_string( v ) ); }
bool SimpleArchive::operator<<( double v ) { return Put( mData, std::to_string( v ) ); }
bool SimpleArchive::operator<<( bool v ) { return Put( mData, v ? "1" : "0" ); }
bool SimpleArchive::operator<<( const char* v ) { return Put( mData, v ); }
bool SimpleArchive::operator<<( Channel& c ) { return Put( mData, std::to_string( c.mDeviceId ) + " " + std::to_string( c.mChannelIndex ) + " " + std::to_string( (int)c.mDataType ) ); }
static bool Get( SimpleArchiveData* d, std::string& s ) { if( d->rd >= d->toks.size() ) return false; s = d->toks[ d->rd++ ]; return true; }
bool SimpleArchive::operator>>( U64& v ) { std::string s; if( !Get( mData, s ) ) return false; v = std::stoull( s ); return true; }
bool SimpleArchive::operator>>( U32& v ) { std::string s; if( !Get( mData, s ) ) return false; v = (U32)std::stoul( s ); return true; }
bool SimpleArchive::operator>>( S64& v ) { std::string s; if( !Get( mData, s ) ) return false; v = std::stoll( s ); return true; }
bool SimpleArchive::operator>>( S32& v ) { std::string s; if( !Get( mData, s ) ) return false; v = std::stoi( s ); return true; }
bool SimpleArchive::operator>>( double& v ) { std::string s; if( !Get( mData, s ) ) return false; v = std::stod( s ); return true; }
bool SimpleArchive::operator>>( bool& v ) { std::string s; if( !Get( mData, s ) ) return false; v = s == "1"; return true; }
bool SimpleArchive::operator>>( char const** v ) { std::string s; if( !Get( mData, s ) ) return false; static std::string keep; keep = s; *v = keep.c_str(); return true; }
bool SimpleArchive::operator>>( Channel& c ) { U64 d; U32 i; U32 t; if( !( *this >> d ) || !( *this >> i ) || !( *this >> t ) ) return false; c = Channel( d, i, (ChannelDataType)t ); return true; }
