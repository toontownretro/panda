// Filename: pnmFileTypeSGIWriter.cxx
// Created by:  drose (17Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "pnmFileTypeSGI.h"
#include "config_pnmimagetypes.h"
#include "sgi.h"

#include <pnmImage.h>
#include <pnmWriter.h>

// Much code in this file originally came from from Netpbm,
// specifically pnmtosgi.c.  It has since been fairly heavily
// modified.

/* pnmtosgi.c - convert portable anymap to SGI image
**
** Copyright (C) 1994 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
**
** Based on the SGI image description v0.9 by Paul Haeberli (paul@sgi.comp)
** Available via ftp from sgi.com:graphics/SGIIMAGESPEC
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** 29Jan94: first version
*/



#define WORSTCOMPR(x)   (2*(x) + 2)


#define MAXVAL_BYTE     255
#define MAXVAL_WORD     65535

inline void 
put_byte(FILE *out_file, unsigned char b) {
  putc(b, out_file);
}

static void
put_big_short(FILE *out_file, short s) {
    if ( pm_writebigshort( out_file, s ) == -1 )
        pm_error( "write error" );
}


static void
put_big_long(FILE *out_file, long l) {
    if ( pm_writebiglong( out_file, l ) == -1 )
        pm_error( "write error" );
}


static void
put_short_as_byte(FILE *out_file, short s) {
    put_byte(out_file, (unsigned char)s);
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeSGI::Writer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeSGI::Writer::
Writer(PNMFileType *type, FILE *file, bool owns_file) :
  PNMWriter(type, file, owns_file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeSGI::Writer::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PNMFileTypeSGI::Writer::
~Writer() {
  if (table!=NULL) {
    // Rewrite the table with the correct values in it.
    fseek(_file, table_start, SEEK_SET);
    write_table();
    delete[] table;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeSGI::Writer::supports_write_row
//       Access: Public, Virtual
//  Description: Returns true if this particular PNMWriter supports a
//               streaming interface to writing the data: that is, it
//               is capable of writing the image one row at a time,
//               via repeated calls to write_row().  Returns false if
//               the only way to write from this file is all at once,
//               via write_data().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeSGI::Writer::
supports_write_row() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeSGI::Writer::write_header
//       Access: Public, Virtual
//  Description: If supports_write_row(), above, returns true, this
//               function may be called to write out the image header
//               in preparation to writing out the image data one row
//               at a time.  Returns true if the header is
//               successfully written, false if there is an error.
//
//               It is the user's responsibility to fill in the header
//               data via calls to set_x_size(), set_num_channels(),
//               etc., or copy_header_from(), before calling
//               write_header().
////////////////////////////////////////////////////////////////////
bool PNMFileTypeSGI::Writer::
write_header() {
  table = NULL;

  switch (_num_channels) {
  case 1:
  case 2:
    dimensions = 2;
    break;

  case 3:
  case 4:
    dimensions = 3;
    dimensions = 3;
    break;

  default:
    nassertr(false, false);
  }

  // For some reason, we have problems with SGI image files whose pixmax value
  // is not 255 or 65535.  So, we'll round up when writing.
  if( _maxval <= MAXVAL_BYTE ) {
    bpc = 1;
    new_maxval = MAXVAL_BYTE;
  } else if( _maxval <= MAXVAL_WORD ) {
    bpc = 2;
    new_maxval = MAXVAL_WORD;
  } else {
    return false;
  }
  
  if( sgi_storage_type != STORAGE_VERBATIM ) {
    table = new TabEntry[_num_channels * _x_size];
    memset(table, 0, _num_channels * _x_size * sizeof(TabEntry));
  }

  write_rgb_header(sgi_imagename.c_str());

  if (table!=NULL) {
    table_start = ftell(_file);

    // The first time we write the table, it has zeroes.  We'll correct
    // this later.
    write_table();
  }

  current_row = _y_size - 1;
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PNMFileTypeSGI::Writer::write_row
//       Access: Public, Virtual
//  Description: If supports_write_row(), above, returns true, this
//               function may be called repeatedly to write the image,
//               one horizontal row at a time, beginning from the top.
//               Returns true if the row is successfully written,
//               false if there is an error.
//
//               You must first call write_header() before writing the
//               individual rows.  It is also important to delete the
//               PNMWriter class after successfully writing the last
//               row.  Failing to do this may result in some data not
//               getting flushed!
////////////////////////////////////////////////////////////////////
bool PNMFileTypeSGI::Writer::
write_row(xel *row_data, xelval *alpha_data) {
  ScanLine channel[4];

  build_scanline(channel, row_data, alpha_data);

  if( bpc == 1 )
    write_channels(channel, put_short_as_byte);
  else
    write_channels(channel, put_big_short);
  
  for (int i = 0; i < _num_channels; i++) {
    delete[] channel[i].data;
  }

  current_row--;
  return true;
}


void PNMFileTypeSGI::Writer::
write_rgb_header(const char *imagename) {
    int i;

    put_big_short(_file, SGI_MAGIC);
    put_byte(_file, sgi_storage_type);
    put_byte(_file, (char)bpc);
    put_big_short(_file, dimensions);
    put_big_short(_file, _x_size);
    put_big_short(_file, _y_size);
    put_big_short(_file, _num_channels);
    put_big_long(_file, 0);                /* PIXMIN */
    put_big_long(_file, new_maxval);           /* PIXMAX */
    for( i = 0; i < 4; i++ )
        put_byte(_file, 0);
    for( i = 0; i < 79 && imagename[i] != '\0'; i++ )
        put_byte(_file, imagename[i]);
    for(; i < 80; i++ )
        put_byte(_file, 0);
    put_big_long(_file, CMAP_NORMAL);
    for( i = 0; i < 404; i++ )
        put_byte(_file, 0);
}


void PNMFileTypeSGI::Writer::
write_table() {
    int i;
    int tabsize = _y_size*_num_channels;

    for( i = 0; i < tabsize; i++ ) {
        put_big_long(_file, table[i].start);
    }
    for( i = 0; i < tabsize; i++ )
        put_big_long(_file, table[i].length);
}


void PNMFileTypeSGI::Writer::
write_channels(ScanLine channel[], void (*put)(FILE *, short)) {
  int i, col;
  
  for( i = 0; i < _num_channels; i++ ) {
    Table(i).start = ftell(_file);
    Table(i).length = channel[i].length * bpc;

    for( col = 0; col < channel[i].length; col++ ) {
      (*put)(_file, channel[i].data[col]);
    }
  }
}


void PNMFileTypeSGI::Writer::
build_scanline(ScanLine output[], xel *row_data, xelval *alpha_data) {
  int col;
  ScanElem *temp;
  
  if( sgi_storage_type != STORAGE_VERBATIM ) {
    rletemp = (ScanElem *)alloca(WORSTCOMPR(_x_size) * sizeof(ScanElem));
  }
  temp = new ScanElem[_x_size];
  
  if( _num_channels <= 2 ) {
    for( col = 0; col < _x_size; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETB(row_data[col]) / _maxval);
    temp = compress(temp, output[0]);

    if (_num_channels == 2) {
      for( col = 0; col < _x_size; col++ )
	temp[col] = (ScanElem)
	  (new_maxval * alpha_data[col] / _maxval);
      temp = compress(temp, output[1]);
    }

  } else {
    for( col = 0; col < _x_size; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETR(row_data[col]) / _maxval);
    temp = compress(temp, output[0]);
    for( col = 0; col < _x_size; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETG(row_data[col]) / _maxval);
    temp = compress(temp, output[1]);
    for( col = 0; col < _x_size; col++ )
      temp[col] = (ScanElem)
	(new_maxval * PPM_GETB(row_data[col]) / _maxval);
    temp = compress(temp, output[2]);
    if (_num_channels == 4) {
      for( col = 0; col < _x_size; col++ )
	temp[col] = (ScanElem)
	  (new_maxval * alpha_data[col] / _maxval);
      temp = compress(temp, output[3]);
    }
  }

  delete[] temp;
}


PNMFileTypeSGI::Writer::ScanElem *PNMFileTypeSGI::Writer::
compress(ScanElem *temp, ScanLine &output) {
    int len;

    switch( sgi_storage_type ) {
        case STORAGE_VERBATIM:
            output.length = _x_size;
            output.data = temp;
            temp = new ScanElem[_x_size];
            break;
        case STORAGE_RLE:
            len = rle_compress(temp, _x_size);    /* writes result into rletemp */
            output.length = len;
            output.data = new ScanElem[len];
	    memcpy(output.data, rletemp, len * sizeof(ScanElem));
            break;
        default:
            pm_error("unknown storage type - can\'t happen");
    }
    return temp;
}


/*
slightly modified RLE algorithm from ppmtoilbm.c
written by Robert A. Knop (rknop@mop.caltech.edu)
*/
int PNMFileTypeSGI::Writer::
rle_compress(ScanElem *inbuf, int size) {
    int in, out, hold, count;
    ScanElem *outbuf = rletemp;

    in=out=0;
    while( in<size ) {
        if( (in<size-1) && (inbuf[in]==inbuf[in+1]) ) {     /*Begin replicate run*/
            for( count=0,hold=in; in<size && inbuf[in]==inbuf[hold] && count<127; in++,count++)
                ;
            outbuf[out++]=(ScanElem)(count);
            outbuf[out++]=inbuf[hold];
        }
        else {  /*Do a literal run*/
            hold=out; out++; count=0;
            while( ((in>=size-2)&&(in<size)) || ((in<size-2) && ((inbuf[in]!=inbuf[in+1])||(inbuf[in]!=inbuf[in+2]))) ) {
                outbuf[out++]=inbuf[in++];
                if( ++count>=127 )
                    break;
            }
            outbuf[hold]=(ScanElem)(count | 0x80);
        }
    }
    outbuf[out++] = (ScanElem)0;     /* terminator */
    return(out);
}

