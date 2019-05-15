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
 
#ifndef STRINGCONVERSIONS_H
#define STRINGCONVERSIONS_H
#include <string>
#include <vector>

#ifdef _WIN32
#include <stringapiset.h>
extern std::string ConvertFromUtf16ToUtf8(const wchar_t * wstr);
#endif

#endif //STRINGCONVERSIONS_H