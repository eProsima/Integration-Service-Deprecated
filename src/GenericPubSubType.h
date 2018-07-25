// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _GENERIC_PUBSUBTYPES_H_
#define _GENERIC_PUBSUBTYPES_H_

#include <string>
#include <fastrtps/TopicDataType.h>

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

class GenericPubSubType : public TopicDataType 
{
public:
    GenericPubSubType();
    virtual ~GenericPubSubType();
    bool serialize(void *data, SerializedPayload_t *payload);
    bool deserialize(SerializedPayload_t *payload, void *data);
    std::function<uint32_t()> getSerializedSizeProvider(void* data);
    void* createData() { return 0; }
    void deleteData(void*){ }
    MD5 m_md5;
    unsigned char* m_keyBuffer;
};

#endif // _GENERIC_PUBSUBTYPE_H_
