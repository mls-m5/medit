
namespace apspace {

class Apa {
public:
    int length;
};

} // namespace apspace

int main(int argc, char **argv) {
    apspace::Apa apa;

    apa.length = argv[argc][0];
}
