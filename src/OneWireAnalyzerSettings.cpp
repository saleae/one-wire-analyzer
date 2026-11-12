#include "OneWireAnalyzerSettings.h"

#include "OneWireAnalyzer.h" // sample point defaults
#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

OneWireAnalyzerSettings::OneWireAnalyzerSettings() : mOneWireChannel( UNDEFINED_CHANNEL ), mOverdrive( false ),
    mStdSamplePointNs( DEFAULT_SAMPLE_POINT_NS ), mOvdSamplePointNs( DEFAULT_OVD_SAMPLE_POINT_NS )
{
    mOneWireChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mOneWireChannelInterface->SetTitleAndTooltip( "1-Wire", "Maxim 1-Wire Interface" );
    mOneWireChannelInterface->SetChannel( mOneWireChannel );

    mOverDriveInterface.reset( new AnalyzerSettingInterfaceBool() );
    mOverDriveInterface->SetTitleAndTooltip( "", "The analyzer will only operate at high speed overdrive mode." );
    mOverDriveInterface->SetCheckBoxText( "Overdrive only mode" );
    mOverDriveInterface->SetValue( false );

    mStdSamplePointInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mStdSamplePointInterface->SetTitleAndTooltip(
        "Standard speed sample point (ns)",
        "Time after the falling edge of a read slot at which the bit is decided. Low = 0, released = 1. Keep it inside t_RDV (15 us)." );
    mStdSamplePointInterface->SetMin( MIN_SAMPLE_POINT_NS );
    mStdSamplePointInterface->SetMax( MAX_SAMPLE_POINT_NS );
    mStdSamplePointInterface->SetInteger( mStdSamplePointNs );

    mOvdSamplePointInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mOvdSamplePointInterface->SetTitleAndTooltip(
        "Overdrive sample point (ns)",
        "Time after the falling edge of a read slot at which the bit is decided. Low = 0, released = 1. Keep it inside t_RDV (2 us)." );
    mOvdSamplePointInterface->SetMin( MIN_SAMPLE_POINT_NS );
    mOvdSamplePointInterface->SetMax( MAX_SAMPLE_POINT_NS );
    mOvdSamplePointInterface->SetInteger( mOvdSamplePointNs );

    AddInterface( mOneWireChannelInterface.get() );
    AddInterface( mOverDriveInterface.get() );
    AddInterface( mStdSamplePointInterface.get() );
    AddInterface( mOvdSamplePointInterface.get() );

    // AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mOneWireChannel, "1-WIRE", false );
}

OneWireAnalyzerSettings::~OneWireAnalyzerSettings()
{
}

void OneWireAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mOneWireChannelInterface->SetChannel( mOneWireChannel );
    mOverDriveInterface->SetValue( mOverdrive );
    mStdSamplePointInterface->SetInteger( mStdSamplePointNs );
    mOvdSamplePointInterface->SetInteger( mOvdSamplePointNs );
}

bool OneWireAnalyzerSettings::SetSettingsFromInterfaces()
{
    mOneWireChannel = mOneWireChannelInterface->GetChannel();
    mOverdrive = mOverDriveInterface->GetValue();
    mStdSamplePointNs = U32( mStdSamplePointInterface->GetInteger() );
    mOvdSamplePointNs = U32( mOvdSamplePointInterface->GetInteger() );

    ClearChannels();
    AddChannel( mOneWireChannel, "1-WIRE", true );

    return true;
}

void OneWireAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    const char* name_string; // the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if( strcmp( name_string, "SaleaeOneWireAnalyzer" ) != 0 )
        AnalyzerHelpers::Assert( "SaleaeOneWireAnalyzer: Provided with a settings string that doesn't belong to us;" );

    text_archive >> mOneWireChannel;
    text_archive >> mOverdrive;

    // Sample points were added later; settings saved by older versions end after mOverdrive, so keep the defaults then.
    mStdSamplePointNs = DEFAULT_SAMPLE_POINT_NS;
    mOvdSamplePointNs = DEFAULT_OVD_SAMPLE_POINT_NS;
    U32 std_ns, ovd_ns;
    if( ( text_archive >> std_ns ) && ( text_archive >> ovd_ns ) )
    {
        mStdSamplePointNs = std_ns;
        mOvdSamplePointNs = ovd_ns;
    }

    ClearChannels();
    AddChannel( mOneWireChannel, "1-WIRE", true );

    UpdateInterfacesFromSettings();
}

const char* OneWireAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "SaleaeOneWireAnalyzer";
    text_archive << mOneWireChannel;
    text_archive << mOverdrive;
    text_archive << mStdSamplePointNs;
    text_archive << mOvdSamplePointNs;

    return SetReturnString( text_archive.GetString() );
}
