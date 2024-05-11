/**
 * --------------
 * Compress Script to convert SPAs to embedded microcontroller compatible format
 * --------------
 * This is an property of SOFTT Product Design Studio - www.softt.io and holds
 * a restrictive GPL-3.0 license. Any use of this code directly or indirectly
 * in commerical projects must be licensed by SOFTT.
 * 
 * Contact us at support@softt.io for commercial licensing.
 */

import FS from 'fs'
import path from 'path'
import { gzipAsync } from '@gfx/zopfli';

const SAVE_PATH = '../src';

// Check if save path exists
if(!FS.existsSync(SAVE_PATH)){
  throw new Error(`Save path ${SAVE_PATH} does not exist`);
}

const INDEX_HTML = FS.readFileSync(path.resolve(path.resolve(), './dist/index.html'));

function chunkArray(myArray, chunk_size){
  var index = 0;
  var arrayLength = myArray.length;
  var tempArray = [];
  for (index = 0; index < arrayLength; index += chunk_size) {
      let myChunk = myArray.slice(index, index+chunk_size);
      // Do something if you want with the group
      tempArray.push(myChunk);
  }
  return tempArray;
}
 
function addLineBreaks(buffer){
   let data = '';
   let chunks = chunkArray(buffer, 30);
   chunks.forEach((chunk, index) => {
     data += chunk.join(',');
     if(index+1 !== chunks.length){
       data+=',\n';
     }
   });
   return data;
 }
 
(async () => {
  try{
    const GZIPPED_INDEX = await gzipAsync(INDEX_HTML, { numiterations: 15 });

    const HEADER_FILE = 
`#ifndef elop_h
#define elop_h

#include <Arduino.h>

extern const uint8_t ELEGANT_HTML[${GZIPPED_INDEX.length}];

#endif
`;

   const CPP_FILE =
`#include "elop.h" 

const uint8_t ELEGANT_HTML[${GZIPPED_INDEX.length}] PROGMEM = { 
${ addLineBreaks(GZIPPED_INDEX) }
};
`;

    FS.writeFileSync(path.resolve(path.resolve(), SAVE_PATH+'/elop.h'), HEADER_FILE);
    FS.writeFileSync(path.resolve(path.resolve(), SAVE_PATH+'/elop.cpp'), CPP_FILE);
    console.log(`[COMPRESS.js] Compressed Bundle into elop.h header file | Total Size: ${(GZIPPED_INDEX.length / 1024).toFixed(2) }KB`)
  }catch(err){
    return console.error(err);
  }
})();