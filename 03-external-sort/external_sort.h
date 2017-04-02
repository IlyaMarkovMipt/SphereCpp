#include<vector>
#include<string>
#include<iterator>

template<typename T>
class BinaryStreamWalker {
    std::istreambuf_iterator<char> it;
    std::istreambuf_iterator<char> end;
    std::vector<char> data;
public:
    BinaryStreamWalker(std::ifstream& stream)
      : it(stream),
        end(),
        data(sizeof(T) / sizeof(char))
    {}
    const T& value() const
    {
        return *reinterpret_cast<T const * const>(& data[0]);
    }
    bool move() {
        for (auto i = 0; i < data.size(); ++i) {
            if (it == end) {
                return false;
            }
            else {
                data[i] = *it;
                ++it;
            }
        }
        return true;
    }
};

const int DEFAULT_CHUNK_NUMBER = 100;
// need maximal number of opened files simultaiously, and size of buffer of reading.
void sort(const std::string&, const std::string&, int max_chunk);
void sort_chunk(const std::string& chunk_filename, unsigned int chunk_size);
void merge_sorted_chunks(const std::vector<std::string>& chunk_filenames, 
				std::string output_filename);
std::uint32_t split_into_chunks(const std::string& filename, 
					unsigned int chunk_size);
std::string get_chunk_filename(unsigned int chunk_no);
void delete_files(const std::vector<std::string>& filenames);
