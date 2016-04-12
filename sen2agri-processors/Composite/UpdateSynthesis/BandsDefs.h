/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#ifndef BANDSDEFS_H
#define BANDSDEFS_H

#define S2_L2A_10M_BANDS_NO     4
#define L8_L2A_10M_BANDS_NO     3
#define SPOT4_L2A_10M_BANDS_NO  3

#define WEIGHTED_REFLECTANCE_10M_BANDS_NO   S2_L2A_10M_BANDS_NO

//S2 10m Bands defines
#define S2_L2A_10M_BLUE_BAND_IDX        0
#define S2_L2A_10M_RED_BAND_IDX         2

//Landsat 8 10m Bands defines
#define L8_L2A_10M_BLUE_BAND_IDX        0
#define L8_L2A_10M_RED_BAND_IDX         2

//SPOT4 10m Bands defines
// For SPOT4 the blue band is actually the green band
#define SPOT4_L2A_10M_BLUE_BAND_IDX        0
#define SPOT4_L2A_10M_RED_BAND_IDX         1

// 20M Positions Definition
#define S2_L2A_20M_BANDS_NO     6
#define L8_L2A_20M_BANDS_NO     3
#define SPOT4_L2A_20M_BANDS_NO  1
#define WEIGHTED_REFLECTANCE_20M_BANDS_NO   S2_L2A_20M_BANDS_NO

//S2 20m Bands defines
#define S2_L2A_20M_BLUE_BAND_IDX        -1
#define S2_L2A_20M_RED_BAND_IDX         -1

//Landsat 8 20m Bands defines
#define L8_L2A_20M_BLUE_BAND_IDX        -1
#define L8_L2A_20M_RED_BAND_IDX         -1

//SPOT4 20m Bands defines
#define SPOT4_L2A_20M_BLUE_BAND_IDX        -1
#define SPOT4_L2A_20M_RED_BAND_IDX         -1

// These defines are for the case when all the bands of 10 AND 20m are resampled at the specified resolution
// and are all present

#define S2_L2A_ALL_BANDS_NO     10
#define L8_L2A_ALL_BANDS_NO     6
#define SPOT4_L2A_ALL_BANDS_NO  4

#define WEIGHTED_REFLECTANCE_ALL_BANDS_NO   S2_L2A_ALL_BANDS_NO

//S2 Bands defines
#define S2_L2A_ALL_BLUE_BAND_IDX        0
#define S2_L2A_ALL_RED_BAND_IDX         2

//Landsat 8 Bands defines
#define L8_L2A_ALL_BLUE_BAND_IDX        0
#define L8_L2A_ALL_RED_BAND_IDX         2

//SPOT4 Bands defines
// For SPOT4 the blue band is actually the green band
#define SPOT4_L2A_ALL_BLUE_BAND_IDX        0
#define SPOT4_L2A_ALL_RED_BAND_IDX         1

#define L3A_WEIGHTED_REFLECTANCES_MAX_NO       S2_L2A_ALL_BANDS_NO

#endif // BANDSDEFS_H

