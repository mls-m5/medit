#pragma once

#include "text/fuzzymatch.h"
#include "views/editor.h"
#include "views/listview.h"
#include <filesystem>

/// One row of a quick list.
struct QuickListItem {
    FString text;             // Shown and matched; its formatting is kept
    std::string value;        // Passed to the callback when selected
    size_t emphasisStart = 0; // Matches from here score higher (file names)
    FString hint;             // Shown right-aligned and not matched
};

/// A search field with a list below it. The list is filtered with fuzzy
/// matching as the user types; the best matches come first.
class QuickList : public Editor {
public:
    using PopulateRetT = std::vector<QuickListItem>;
    using PopulateFunctionT = std::function<PopulateRetT()>;

    /// At most this many rows are shown.
    static constexpr size_t maxShown = 200;

    QuickList(const QuickList &) = delete;
    QuickList(QuickList &&) = delete;
    QuickList &operator=(const QuickList &) = delete;
    QuickList &operator=(QuickList &&) = delete;
    QuickList(IView *parent, PopulateFunctionT);
    ~QuickList() override;

    // @see IKeySink
    bool keyPress(std::shared_ptr<IEnvironment>) override;

    // @see IView
    void draw(IScreen &) override;

    void callback(std::function<void(std::filesystem::path)> f);

    void visible(bool value);

    bool visible() const;

    /// Reads the items again and filters them with the current text.
    void updateList();

    /// Filters the items read by the last updateList().
    void filter();

    /// The rows currently shown, best first, as their values.
    std::vector<std::string> shownValues() const;

private:
    ListView _list;
    PopulateFunctionT _populate;
    PopulateRetT _items;
    std::vector<FuzzyCandidate> _candidates;
    std::vector<std::string> _shownValues;
};
