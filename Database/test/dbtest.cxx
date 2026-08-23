// Test program for the Database system. Exercises multiple-save scenarios.
// Built separately via Database/test/Makefile-test so it doesn't get linked
// into the main RPG binary.

#include <cstring>
#include <iostream>
#include <string>

#include "../header.hxx"

[[maybe_unused]] static void dump(const std::string& tag) {
  std::cout << "=== " << tag << " ===\n";
}

int main(int argc, char** argv) {
  std::string op = argc > 1 ? argv[1] : "init";

  DB::Database db("DBTest");
  DB::Schema s;
  s.setIndex("test")
    .addField("score", DB::Schema::integer)
    .addField("name", DB::Schema::text);
  db.add("scorecard", &s);

  std::cout << "Loaded tables: " << db.tables.size() << "\n";

  if (op == "init") {
    DB::Row& r1 = db["scorecard"].row("player1");
    int v = 42;
    r1["score"].set(DB::Schema::integer, &v);
    r1.save();
    std::cout << "Wrote player1=42\n";

    DB::Row& r2 = db["scorecard"].row("player2");
    int v2 = 100;
    r2["score"].set(DB::Schema::integer, &v2);
    r2.save();
    std::cout << "Wrote player2=100\n";
  } else if (op == "load") {
    DB::Row* r1 = db["scorecard"]["player1"];
    if (!r1) { std::cout << "no player1\n"; return 1; }
    int* p = (int*)((*r1)["score"].get());
    std::cout << "player1.score = " << (p ? *p : -1) << "\n";

    DB::Row* r2 = db["scorecard"]["player2"];
    if (r2) {
      int* p2 = (int*)((*r2)["score"].get());
      std::cout << "player2.score = " << (p2 ? *p2 : -1) << "\n";
    } else {
      std::cout << "no player2\n";
    }
  } else if (op == "rewrite") {
    DB::Row* r = db["scorecard"]["player1"];
    if (!r) { std::cout << "no player1\n"; return 1; }
    int* p = (int*)((*r)["score"].get());
    std::cout << "Loaded player1.score = " << *p << "\n";
    for (int i = 0; i < 5; i++) {
      *p = 1000 + i;
      r->save();
      std::cout << "Saved iteration " << i << " value=" << *p << "\n";
    }
  } else if (op == "append") {
    DB::Row& r = db["scorecard"].row("player3");
    int v = 999;
    r["score"].set(DB::Schema::integer, &v);
    r.save();
    std::cout << "Wrote player3=999\n";
  } else if (op == "multifield") {
    DB::Row& r = db["scorecard"].row("playerA");
    int score = 7;
    r["score"].set(DB::Schema::integer, &score);
    const char* name = "Alice";
    r["name"].set(DB::Schema::text, const_cast<char*>(name));
    r.save();
    std::cout << "Wrote playerA score=7 name=Alice\n";

    // Save again to test re-save with same fields.
    score = 8;
    r.save();
    std::cout << "Re-saved playerA score=8\n";

    // Append a different row with both fields.
    DB::Row& r2 = db["scorecard"].row("playerB");
    int s2 = 3;
    r2["score"].set(DB::Schema::integer, &s2);
    const char* n2 = "Bob";
    r2["name"].set(DB::Schema::text, const_cast<char*>(n2));
    r2.save();
    std::cout << "Wrote playerB score=3 name=Bob\n";
  } else if (op == "reloadmultifield") {
    DB::Row* r = db["scorecard"]["playerA"];
    if (!r) { std::cout << "no playerA\n"; return 1; }
    int* sp = (int*)((*r)["score"].get());
    char* np = (char*)((*r)["name"].get());
    std::cout << "playerA.score = " << (sp ? *sp : -1) << "\n";
    std::cout << "playerA.name = " << (np ? np : "(null)") << "\n";

    DB::Row* r2 = db["scorecard"]["playerB"];
    if (r2) {
      int* sp2 = (int*)((*r2)["score"].get());
      char* np2 = (char*)((*r2)["name"].get());
      std::cout << "playerB.score = " << (sp2 ? *sp2 : -1) << "\n";
      std::cout << "playerB.name = " << (np2 ? np2 : "(null)") << "\n";
    } else {
      std::cout << "no playerB\n";
    }
  }

  return 0;
}
