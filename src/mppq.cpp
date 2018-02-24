/*
	Copyright (C) 2018 by Claude SIMON (http://zeusw.org/epeios/contact.html).

	This file is part of 'MPPq'.

    'MPPq' is free software: you can redistribute it and/or modify it
    under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    'MPPq' is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with 'MPPq'.  If not, see <http://www.gnu.org/licenses/>.
*/

# include "mppq.h"

#include "registry.h"

#include "scltool.h"
#include "sclerror.h"

#include "err.h"
#include "cio.h"
#include "epsmsc.h"
#include "xpp.h"
#include "fnm.h"
#include "flf.h"
#include "crt.h"

using cio::CErr;
using cio::COut;
using cio::CIn;

namespace {
	typedef str::dStrings dSlide;
	qW( Slide );

	void PrintHeader_( void )
	{
		COut << NAME_MC " V" VERSION << " (" WEBSITE_URL ")" << txf::nl;
		COut << "Copyright (C) " COPYRIGHT << txf::nl;
		COut << txf::pad << "Build : " __DATE__ " " __TIME__ << " (" << cpe::GetDescription() << ')' << txf::nl;
	}

	void Display_(
		sdr::sRow Last,
		const dSlide &Slide,
		txf::sWFlow &Flow )
	{
		sdr::sRow Row = Slide.First();

		if( Last == Row )
			Flow << Slide( Row );
		else {
			while ( Row != Last ) {
				Flow << Slide( Row ) << txf::nl;

				Row = Slide.Next( Row );
			}
		}
	}

	bso::sBool IsListItem_( const str::dString &Line )
	{
		bso::sBool Is = false;
	qRH;
		flx::sStringRFlow Flow;
		bso::sBool DigitFound = false;
		bso::sBool SpaceAwaited = false;
		bso::sBool Continue = true;
	qRB;
		Flow.Init( Line );

		while ( Continue && !Flow.EndOfFlow() ) {
			if ( SpaceAwaited ) {
				Continue = false;
				Is = Flow.Get() == ' ';
			} else if ( DigitFound ) {
				char C = Flow.Get();
				
				if ( !isdigit( C ) ) {
					if ( C == '.' )
						SpaceAwaited = true;
					else
						Continue = false;
				}
			} else {
				char C = Flow.Get();

				if ( isdigit( C ) )
					DigitFound = true;
				else if ( C == '*' )
					SpaceAwaited = true;
				else if ( C != ' ' )
					Continue = false;
			}
		}
	qRR;
	qRT;
	qRE;
		return Is;
	}

	void Handle_(
		const str::dStrings &Slide,
		txf::sWFlow &Flow )
	{
		sdr::sRow Row = Slide.First();

		while ( Row != qNIL ) {
			if (IsListItem_(Slide(Row))) {
				Display_(Row, Slide, Flow);

				// 'Marp' slide separator.
				Flow << "---" << txf::nl;
			}

			Row = Slide.Next( Row );
		}

		Display_( qNIL, Slide, Flow );
	}

	void Process_(
		flw::sRFlow &RFlow,
		txf::sWFlow &WFlow )
	{
	qRH;
		xtf::sIFlow Flow;
		str::wString Line;
		wSlide Slide;
		bso::sBool Continue = true;
	qRB;
		Flow.Init( RFlow, utf::f_Guess );

		Slide.Init();

		while ( Continue ) {
			if ( Flow.EndOfFlow() )
				Continue = false;
			else {
				Line.Init();
				Flow.GetLine( Line );

				Slide.Append( Line );

				if ( Line == "---" ) {
					Handle_( Slide, WFlow );

					Slide.Init();
				}
			}
		}

		if ( Slide.Amount() != 0 )
			Handle_( Slide, WFlow );
	qRR;
	qRT;
	qRE;
	}

	void Process_(
		const char *Source,
		const char *Target )
	{
	qRH;
		flf::rRFlow SFlow;
		flf::rWDriver TFDriver;	// Target file driver.
		txf::rWFlow TTFlow;	// Target text flow.
		bso::sBool BackedUp = false;
	qRB;
		if ( Source != NULL ) {
			if ( SFlow.Init( Source, err::hUserDefined ) != tol::rSuccess )
				sclmisc::ReportFileOpeningErrorAndAbort( Source );
		}

		if ( Target != NULL ) {
			sclmisc::CreateBackupFile( Target );

			BackedUp = true;

			if ( TFDriver.Init( Target, err::hUserDefined ) != tol::rSuccess )
				sclmisc::ReportFileOpeningErrorAndAbort( Target );

			TTFlow.Init( TFDriver );
		}


		Process_(
			Source == NULL ? CIn.Flow() : SFlow,
			Target == NULL ? COut : TTFlow );
	qRR;
		if ( BackedUp ) {
			tol::reset( TFDriver, TTFlow );
			sclmisc::RecoverBackupFile( Target );
		}
	qRT;
	qRE;
	}

	void Process_( void )
	{
	qRH;
		str::wString Input, Output;
		qCBUFFERr InputBuffer, OutputBuffer;
	qRB;
		tol::Init( Input );
		sclmisc::OGetValue( registry::parameter::Input, Input );

		tol::Init( Output );
		sclmisc::OGetValue( registry::parameter::Output, Output );

		Process_(
			Input.Amount() != 0 ? Input.Convert( InputBuffer ) : NULL,
			Output.Amount() != 0 ? Output.Convert( OutputBuffer ) : NULL );
	qRR;
	qRT;
	qRE;
	}
}

#define C( name )\
	else if ( Command == #name )\
		name##_()

int scltool::SCLTOOLMain(
	const str::dString &Command,
	const scltool::fOddities &Oddities )
{
	int ExitValue = EXIT_FAILURE;
qRH
qRB
	if ( Command == "Version" )
		PrintHeader_();
	else if ( Command == "License" )
		epsmsc::PrintLicense( NAME_MC );
	C( Process );
	else
		qRGnr();

	ExitValue = EXIT_SUCCESS;
qRR
qRT
qRE
	return ExitValue;
}

const char *sclmisc::SCLMISCTargetName = NAME_LC;
const char *sclmisc::SCLMISCProductName = NAME_MC;

