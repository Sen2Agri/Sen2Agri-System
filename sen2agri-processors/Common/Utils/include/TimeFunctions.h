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
 
#ifndef TIMEFUNCTIONS_H
#define TIMEFUNCTIONS_H

#ifdef _WIN32
#include <time.h>
extern char *strptime (const char *buf, const char *format, struct tm *tm);
extern time_t internal_timegm(tm const *t);
#endif
#endif //TIMEFUNCTIONS_H
