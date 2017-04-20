/*
 * Copyright (c) 2017 Juliette Foucaut
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifdef _MSC_VER
#pragma warning (disable: 4996)
#endif

#include <stdio.h>
#include "enkimi.h"


void PrintStreamStructureToFile( enkiNBTDataStream* pStream_, FILE* fpOutput_ )
{
	if( NULL != pStream_->pCurrPos )
	{
		while( enkiNBTReadNextTag( pStream_ ) )
		{
			for( int i = 0; i <= pStream_->level; i++ )
			{
				fprintf( fpOutput_, "\t" );
			}

			fprintf( fpOutput_, "%s ", enkiGetNBTTagHeaderIDAsString( pStream_->currentTag ) );
			if( pStream_->currentTag.pName != NULL )
			{
				fprintf( fpOutput_, "%s", pStream_->currentTag.pName );
			}
			if( pStream_->level >= 0 )	// if there is a parent
			{
				fprintf( fpOutput_, " Parent = " );
				for( int i = 0; i <= pStream_->level; i++ )
				{
					if( i )
					{
						fprintf( fpOutput_, "." );
					}
					if( pStream_->parentTags[ i ].pName != NULL )
					{
						fprintf( fpOutput_, "%s", pStream_->parentTags[ i ].pName );
					}
				}
			}
			fprintf( fpOutput_, "\n" );
		}
	}
}


int main( int argc, const char * argv[])
{
	int retval = 0;

	// open the region file
	FILE *fp = fopen( "../example/r.1.0.mca", "rb" );
	if( !fp )
	{
		printf( "failed to open file\n" );
		retval = -1;
		return retval;
	}

	// output file
	FILE *fpOutput = fopen( "output.txt", "w" );
	if( !fpOutput )
	{
		printf( "failed to open output file\n" );
		retval = -1;
		return retval;
	}	

	enkiRegionFile regionFile = enkiRegionFileLoad( fp );

	for( int i = 0; i < ENKI_MI_REGION_CHUNKS_NUMBER; i++ )
	{
		enkiNBTDataStream stream;
		enkiInitNBTDataStreamForChunk( regionFile,  i, &stream );
		if( stream.dataLength )
		{
			enkiChunkBlockData aChunk = enkiNBTReadChunk( &stream );
			enkiMICoordinate chunkOriginPos = enkiGetChunkOrigin( &aChunk ); // y always 0
			fprintf( fpOutput, "Chunk at xyz{ %d, %d, %d }  Number of sections: %d \n", chunkOriginPos.x, chunkOriginPos.y, chunkOriginPos.z, aChunk.countOfSections );

			// iterate through chunk and count non 0 voxels as a demo
			int64_t numVoxels = 0;
			for( int section = 0; section < ENKI_MI_NUM_SECTIONS_PER_CHUNK; ++section )
			{
				if( aChunk.sections[ section ] )
				{
					enkiMICoordinate sectionOrigin = enkiGetChunkSectionOrigin( &aChunk, section );
			        fprintf( fpOutput, "    Non empty section at xyz{ %d, %d, %d } \n", sectionOrigin.x, sectionOrigin.y, sectionOrigin.z );
					enkiMICoordinate sPos;
					// note order x then z then y iteration for cache efficiency
					for( sPos.y = 0; sPos.y < ENKI_MI_NUM_SECTIONS_PER_CHUNK; ++sPos.y )
					{
						for( sPos.z = 0; sPos.z < ENKI_MI_NUM_SECTIONS_PER_CHUNK; ++sPos.z )
						{
							for( sPos.x = 0; sPos.x < ENKI_MI_NUM_SECTIONS_PER_CHUNK; ++sPos.x )
							{
								uint8_t voxel = enkiGetChunkSectionVoxel( &aChunk, section, sPos  );
								if( voxel )
								{
									++numVoxels;
								}
							}
						}
					}
				}
			}
			fprintf( fpOutput, "   Chunk has %g non zero voxels\n", (float)numVoxels );

			enkiNBTRewind( &stream );
			PrintStreamStructureToFile( &stream, fpOutput );
		}
		enkiNBTFreeAllocations( &stream );
	}
	
	enkiRegionFileFreeAllocations( &regionFile );

	fclose( fpOutput );
	fclose( fp );

	return retval;
}

