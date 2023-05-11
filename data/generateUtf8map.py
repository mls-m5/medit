import unicodedata

to_lower_map = {}
to_upper_map = {}

for i in range(0x110000):  # 0x110000 is the total number of Unicode code points
    char = chr(i)
    lower_char = char.lower()
    upper_char = char.upper()
    if char != lower_char:
        to_lower_map[char] = lower_char
    if char != upper_char:
        to_upper_map[char] = upper_char

print("auto toLowerMap = std::unordered_map<std::string_view, std::string_view>{")
for key, value in to_lower_map.items():
    print(f'    {{u8"{key}", u8"{value}"}},')
print("};")

print("auto toUpperMap = std::unordered_map<std::string_view, std::string_view>{")
for key, value in to_upper_map.items():
    print(f'    {{u8"{key}", u8"{value}"}},')
print("};")


# With escape sequences does not seem to work
#print("const toLowerMap = std::unordered_map<std::string_view, std::string_view>{")
#for key, value in to_lower_map.items():
#    key = key.encode('unicode_escape').decode()
#    value = value.encode('unicode_escape').decode()
#    print(f'    {{"{key}", "{value}"}},')
#print("};")

#print("const toUpperMap = std::unordered_map<std::string_view, std::string_view>{")
#for key, value in to_upper_map.items():
#    key = key.encode('unicode_escape').decode()
#    value = value.encode('unicode_escape').decode()
#    print(f'    {{"{key}", "{value}"}},')
#print("};")
