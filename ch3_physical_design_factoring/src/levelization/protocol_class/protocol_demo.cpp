// protocol_demo.cpp
#ifndef INCLUDED_DATA_STORE
#include <data_store.h>
#endif

#ifndef INCLUDED_MEMORY_PERSISTENCE
#include <memory_persistence.h>
#endif

#ifndef INCLUDED_FILE_PERSISTENCE
#include <file_persistence.h>
#endif

#include <cassert>
#include <iostream>
#include <string>

static void testWithMemory()
{
    std::cout << "--- Protocol Class with MemoryPersistence ---\n";
    std::cout << "   (Used for testing — no real I/O)\n\n";

    MemoryPersistence storage;  // concrete implementation
    DataStore store(storage);   // DataStore only sees PersistenceAdapter

    store.storeSession("alice", "token-abc-123");
    store.storeSession("bob",   "token-xyz-789");

    assert(store.hasSession("alice"));
    assert(store.hasSession("bob"));
    assert(!store.hasSession("charlie"));

    std::cout << "  Alice session: " << store.getSession("alice") << "\n";
    std::cout << "  Bob session: " << store.getSession("bob") << "\n";
    std::cout << "  Active sessions: " << store.activeSessionCount() << "\n\n";

    store.invalidateSession("alice");
    assert(!store.hasSession("alice"));
    std::cout << "  After invalidating alice: active=" << store.activeSessionCount() << "\n\n";

    std::cout << "  [PASS] MemoryPersistence tests\n\n";
}

static void testWithFile()
{
    std::cout << "--- Protocol Class with FilePersistence ---\n";
    std::cout << "   (Used for production — real file I/O)\n\n";

    const std::string testFile = "/tmp/datastore_test.txt";
    FilePersistence storage(testFile);  // concrete implementation
    DataStore store(storage);           // SAME DataStore, different backend!

    store.storeSession("charlie", "token-file-001");
    assert(store.hasSession("charlie"));
    std::cout << "  Charlie session: " << store.getSession("charlie") << "\n";
    std::cout << "  Active sessions: " << store.activeSessionCount() << "\n\n";

    store.invalidateSession("charlie");
    assert(!store.hasSession("charlie"));
    std::cout << "  After invalidation: " << store.activeSessionCount() << " sessions\n\n";

    std::cout << "  [PASS] FilePersistence tests\n\n";
}

int main()
{
    std::cout << "=== Chapter 3: Protocol Class (Abstract Interface) Demo ===\n\n";

    std::cout << "CONCEPT:\n";
    std::cout << "  DataStore depends ONLY on PersistenceAdapter (abstract).\n";
    std::cout << "  Concrete implementations (Memory, File) are at higher levels.\n";
    std::cout << "  The application wires them together.\n\n";

    std::cout << "DEPENDENCY GRAPH:\n";
    std::cout << "  Application\n";
    std::cout << "    ↓ uses\n";
    std::cout << "  DataStore        (Level 2) — depends on PersistenceAdapter\n";
    std::cout << "  MemoryPersistence (Level 2) — depends on PersistenceAdapter\n";
    std::cout << "  FilePersistence   (Level 2) — depends on PersistenceAdapter\n";
    std::cout << "    ↓ all depend on\n";
    std::cout << "  PersistenceAdapter (Level 1) — abstract interface\n\n";

    std::cout << "BENEFIT:\n";
    std::cout << "  DataStore can be tested with MemoryPersistence (fast, no I/O).\n";
    std::cout << "  In production, swap in FilePersistence or any other impl.\n";
    std::cout << "  DataStore never needs to change when persistence impl changes.\n\n";

    testWithMemory();
    testWithFile();

    std::cout << "=== Key Takeaways ===\n";
    std::cout << "1. Protocol class = pure abstract, no data, virtual destructor\n";
    std::cout << "2. Library code depends on protocol, not concrete implementation\n";
    std::cout << "3. Concrete implementations live at the same or higher level\n";
    std::cout << "4. The application (or test) chooses which impl to inject\n";
    std::cout << "5. This is Dependency Injection via protocol classes\n";

    return 0;
}
