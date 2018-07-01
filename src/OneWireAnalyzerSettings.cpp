#include "OneWireAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

OneWireAnalyzerSettings::OneWireAnalyzerSettings()
:	mOneWireChannel ( UNDEFINED_CHANNEL ),
mOverdrive( false )
{
	mOneWireChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mOneWireChannelInterface->SetTitleAndTooltip( "1-Wire", "Maxim 1-Wire Interface" );
	mOneWireChannelInterface->SetChannel( mOneWireChannel );

	mOverDriveInterface.reset( new AnalyzerSettingInterfaceBool() );
	mOverDriveInterface->SetTitleAndTooltip( "Overdrive only mode", "The analyzer will only operate at high speed overdrive mode." );
	mOverDriveInterface->SetValue( false );

	AddInterface( mOneWireChannelInterface.get() );
	AddInterface( mOverDriveInterface.get() );

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
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
}

bool OneWireAnalyzerSettings::SetSettingsFromInterfaces()
{
	mOneWireChannel = mOneWireChannelInterface->GetChannel();
	mOverdrive = mOverDriveInterface->GetValue();

	ClearChannels();
	AddChannel( mOneWireChannel, "1-WIRE", true );

	return true;
}

void OneWireAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	const char* name_string;	//the first thing in the archive is the name of the protocol analyzer that the data belongs to.
	text_archive >> &name_string;
	if( strcmp( name_string, "SaleaeOneWireAnalyzer" ) != 0 )
		AnalyzerHelpers::Assert( "SaleaeOneWireAnalyzer: Provided with a settings string that doesn't belong to us;" );

	text_archive >>  mOneWireChannel;
	text_archive >> mOverdrive;

	ClearChannels();
	AddChannel( mOneWireChannel, "1-WIRE", true );

	UpdateInterfacesFromSettings();
}

const char* OneWireAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << "SaleaeOneWireAnalyzer";
	text_archive <<  mOneWireChannel;
	text_archive << mOverdrive;

	return SetReturnString( text_archive.GetString() );
}

