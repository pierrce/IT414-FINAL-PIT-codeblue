<script>
  import { onMount } from "svelte";

  let activeRfids = [];
  let logs = [];
  let loading = true;
  let error = null;

  const API_URL = "http://10.197.198.197:8000/api/rfids";

  async function fetchRfids() {
    try {
      loading = true;
      const response = await fetch(API_URL);
      if (!response.ok) throw new Error('Failed to fetch data');

      const allData = await response.json();
      const registeredGroup = {};

      allData.forEach(item => {
        if (item.registered) {
          const current = registeredGroup[item.rfid_number];
          const itemTime = new Date(item.created_at).getTime();
          if (!current || itemTime > new Date(current.created_at).getTime()) {
            registeredGroup[item.rfid_number] = item;
          }
        }
      });

      activeRfids = Object.values(registeredGroup)
        .sort((a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime())
        .slice(0, 5);

      logs = allData.sort((a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime());

      localStorage.setItem('activeRfids', JSON.stringify(activeRfids));
      localStorage.setItem('logs', JSON.stringify(logs));
      localStorage.setItem('lastFetch', new Date().toISOString());

      loading = false;
      error = null;
    } catch (err) {
      console.error('Fetch error:', err);
      const cachedActive = localStorage.getItem('activeRfids');
      const cachedLogs = localStorage.getItem('logs');
      
      if (cachedActive && cachedLogs) {
        activeRfids = JSON.parse(cachedActive);
        logs = JSON.parse(cachedLogs);
        error = 'Offline: ' + formatDate(localStorage.getItem('lastFetch'));
      } else {
        error = 'No connection & no cache';
        activeRfids = [];
        logs = [];
      }
      loading = false;
    }
  }

  function formatDate(dateString) {
    if (!dateString) return 'N/A';
    const date = new Date(dateString);
    const isMobile = window.innerWidth <= 768;
    
    return date.toLocaleString("en-US", {
      month: "short",
      day: "numeric",
      year: "numeric",
      hour: "numeric",
      minute: "2-digit",
      hour12: true
    }).replace(',', isMobile ? ' at' : ',');
  }

  function getStatusDisplay(item) {
    if (!item.registered) return { text: 'RFID NOT FOUND', class: 'notfound' };
    return { text: item.status.toString(), class: item.status === 1 ? 'found' : 'notfound' };
  }

  onMount(() => {
    fetchRfids();
    const interval = setInterval(fetchRfids, 1000);
    return () => clearInterval(interval);
  });
</script>

<main>
  <header class="dashboard-header">
    <h1>
      <span class="codeblue">CodeBlue</span>
      <span class="subtitle-mobile">RFID Monitoring</span>
    </h1>
  </header>

  <div class="container">
    <!-- Registered RFID Panel -->
    <section class="left-panel">
      <div class="card">
        <h3>Registered RFID</h3>
        {#if loading && activeRfids.length === 0}
          <p class="loading-text">Loading...</p>
        {:else if error && activeRfids.length === 0}
          <p class="error">{error}</p>
        {:else}
          {#each activeRfids as item, index}
            <div class="rfid-row">
              <span class="index">{index + 1}.</span>
              <span class="rfid-num">{item.rfid_number}</span>
              <div class="switch disabled">
                <input type="checkbox" checked={item.status === 1} disabled/>
                <span class="slider"></span>
              </div>
            </div>
          {/each}
        {/if}
      </div>
    </section>

    <!-- RFID Logs Panel -->
    <section class="right-panel">
      <div class="card">
        <div class="card-header">
          <h3>RFID Logs</h3>
        </div>

        <!-- Mobile View -->
        <div class="mobile-cards">
          {#if loading && logs.length === 0}
            <div class="empty-state">Loading RFID data...</div>
          {:else if logs.length === 0}
            <div class="empty-state">No RFID records found</div>
          {:else}
            <div class="mobile-header">
              <span>#</span>
              <span>RFID</span>
              <span class="center">Status</span>
              <span class="right">Date & Time</span>
            </div>
            {#each logs as item, i}
              {@const status = getStatusDisplay(item)}
              <div class="log-card">
                <span class="log-number">{i + 1}</span>
                <span class="log-rfid">{item.rfid_number}</span>
                <span class="log-status {status.class}">{@html status.text.replace(' ', '<br>')}</span>
                <span class="log-date">{formatDate(item.created_at)}</span>
              </div>
            {/each}
          {/if}
        </div>

        <!-- Desktop Table View -->
        <div class="table-wrapper">
          <table>
            <thead>
              <tr>
                <th>#</th>
                <th>RFID</th>
                <th>Status</th>
                <th>Date & Time</th>
              </tr>
            </thead>
            <tbody>
              {#if loading && logs.length === 0}
                <tr><td colspan="4" class="center">Loading RFID data...</td></tr>
              {:else if logs.length === 0}
                <tr><td colspan="4" class="center">No RFID records found</td></tr>
              {:else}
                {#each logs as item, i}
                  {@const status = getStatusDisplay(item)}
                  <tr>
                    <td>{i + 1}</td>
                    <td class="rfid-cell">{item.rfid_number}</td>
                    <td><span class={status.class}>{status.text}</span></td>
                    <td class="nowrap">{formatDate(item.created_at)}</td>
                  </tr>
                {/each}
              {/if}
            </tbody>
          </table>
        </div>
      </div>
    </section>
  </div>
</main>

<style>
  :global(body) {
    margin: 0;
    font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
    background: #e8edf2;
    color: #1e293b;
  }

  main {
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 25px;
    min-height: 100vh;
  }

  .dashboard-header {
    text-align: center;
    margin-bottom: 20px;
  }

  .dashboard-header h1 {
    font-size: 1.9rem;
    font-weight: 600;
    color: #1e293b;
    margin: 0;
  }

  .subtitle-mobile { display: inline; }

  .codeblue {
    color: #2563eb;
    font-weight: 800;
    letter-spacing: 1px;
    font-size: 2.2rem;
    text-shadow: 1px 1px 2px rgba(37, 99, 235, 0.2);
  }

  .container {
    display: flex;
    width: 98%;
    max-width: 1800px;
    gap: 25px;
  }

  .card {
    background: #ffffff;
    border-radius: 12px;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
    padding: 24px;
    border: 1px solid #e2e8f0;
  }

  .card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 16px;
  }

  /* Left Panel */
  .left-panel {
    flex: 0 0 280px;
  }

  .left-panel h3 {
    font-size: 1.15rem;
    margin-bottom: 16px;
    color: #2563eb;
    font-weight: 600;
  }

  .rfid-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    background: #f5f7fa;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    margin-bottom: 10px;
    padding: 12px 16px;
    transition: all 0.2s ease;
  }

  .rfid-row:hover {
    background: #eef2f7;
    border-color: #bfdbfe;
    transform: translateY(-1px);
  }

  .rfid-num {
    flex: 1;
    margin-left: 10px;
    font-weight: 500;
    font-size: 0.95rem;
    font-family: 'Courier New', monospace;
  }

  .index {
    font-weight: 700;
    color: #2563eb;
    min-width: 25px;
  }

  .loading-text {
    text-align: center;
    color: #64748b;
    padding: 20px;
  }

  .error {
    color: #dc2626;
    padding: 12px;
    background: #fee2e2;
    border-radius: 6px;
    font-size: 0.9rem;
  }

  /* Switch */
  .switch {
    position: relative;
    display: inline-block;
    width: 46px;
    height: 26px;
    pointer-events: none;
    opacity: 0.85;
  }

  .switch input {
    opacity: 0;
    width: 0;
    height: 0;
  }

  .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #cbd5e1;
    transition: 0.3s;
    border-radius: 26px;
  }

  .slider:before {
    position: absolute;
    content: "";
    height: 20px;
    width: 20px;
    left: 3px;
    bottom: 3px;
    background-color: white;
    transition: 0.3s;
    border-radius: 50%;
    box-shadow: 0 2px 4px rgba(0,0,0,0.2);
  }

  input:checked + .slider {
    background-color: #22c55e;
  }

  input:checked + .slider:before {
    transform: translateX(20px);
  }

  /* Right Panel */
  .right-panel {
    flex: 1;
  }

  .right-panel h3 {
    font-size: 1.15rem;
    margin: 0;
    color: #2563eb;
    font-weight: 600;
  }

  /* Mobile Cards - Hidden on Desktop */
  .mobile-cards { display: none; }

  /* Desktop Table */
  .table-wrapper {
    width: 100%;
    overflow-x: auto;
    display: block;
  }

  table {
    width: 100%;
    border-collapse: collapse;
    font-size: 0.9rem;
  }

  th, td {
    padding: 14px 12px;
    text-align: left;
  }

  th:nth-child(3), td:nth-child(3) {
    text-align: center;
  }

  th {
    background: #2563eb;
    color: white;
    font-weight: 600;
    white-space: nowrap;
  }

  th:first-child { border-top-left-radius: 8px; }
  th:last-child { border-top-right-radius: 8px; }

  td {
    border-bottom: 1px solid #e2e8f0;
  }

  tbody tr { background: #ffffff; }
  tbody tr:nth-child(even) { background: #f9fafb; }
  tbody tr:hover { background: #f0f4f8; }
  tbody tr:last-child td { border-bottom: none; }

  .rfid-cell {
    font-family: 'Courier New', monospace;
    font-weight: 600;
    color: #1e293b;
  }

  .nowrap { white-space: nowrap; }
  .center { text-align: center; }
  .right { text-align: right; }

  .found {
    color: #16a34a;
    font-weight: 600;
  }

  .notfound {
    color: #dc2626;
    font-weight: 600;
  }

  .empty-state {
    text-align: center;
    padding: 40px 20px;
    color: #94a3b8;
    font-style: italic;
  }

  /* Mobile Responsive */
  @media (max-width: 900px) {
    .container {
      flex-direction: column;
      width: 95%;
      gap: 20px;
    }
    .left-panel { width: 100%; }
  }

  @media (max-width: 768px) {
    main { padding: 15px 10px; }

    .dashboard-header h1 {
      font-size: 1.4rem;
      line-height: 1.3;
    }

    .codeblue {
      font-size: 1.6rem;
      display: block;
      margin-bottom: 5px;
    }

    .subtitle-mobile {
      font-size: 1rem;
      font-weight: 500;
      display: block;
    }

    .container {
      width: 100%;
      gap: 15px;
    }

    .card {
      padding: 16px;
      border-radius: 10px;
    }

    .left-panel h3, .right-panel h3 {
      font-size: 1rem;
      margin-bottom: 12px;
    }

    .rfid-row {
      padding: 10px 12px;
      margin-bottom: 8px;
    }

    .rfid-num { font-size: 0.85rem; }

    .switch {
      width: 40px;
      height: 22px;
    }

    .slider:before {
      height: 16px;
      width: 16px;
    }

    input:checked + .slider:before {
      transform: translateX(18px);
    }

    .table-wrapper { display: none; }
    .mobile-cards { display: block; }

    .mobile-header {
      display: grid;
      grid-template-columns: 30px 1fr 60px minmax(100px, auto);
      gap: 8px;
      background: #2563eb;
      color: white;
      padding: 12px;
      font-weight: 600;
      font-size: 0.9rem;
      border-radius: 8px 8px 0 0;
    }

    .log-card {
      display: grid;
      grid-template-columns: 30px 1fr 60px minmax(100px, auto);
      gap: 8px;
      align-items: center;
      background: #ffffff;
      border: 1px solid #e2e8f0;
      border-top: none;
      padding: 14px 12px;
      transition: background 0.2s;
    }

    .log-card:last-child {
      border-radius: 0 0 8px 8px;
    }

    .log-card:active {
      background: #f5f7fa;
    }

    .log-number, .log-rfid {
      font-weight: 600;
      color: #1e293b;
      font-size: 0.9rem;
    }

    .log-rfid {
      font-family: 'Courier New', monospace;
    }

    .log-status {
      text-align: center;
      font-weight: 600;
      font-size: 0.9rem;
      line-height: 1.2;
    }

    .log-date {
      color: #1e293b;
      font-size: 0.8rem;
      text-align: right;
      line-height: 1.4;
    }
  }

  @media (max-width: 400px) {
    .dashboard-header h1 { font-size: 1.2rem; }
    .codeblue { font-size: 1.4rem; }
    .subtitle-mobile { font-size: 0.9rem; }
    .card { padding: 12px; }
    .rfid-row { padding: 8px 10px; }
    .rfid-num { font-size: 0.8rem; }

    .mobile-header, .log-card {
      grid-template-columns: 25px 1fr 50px minmax(85px, auto);
      gap: 6px;
    }

    .mobile-header {
      font-size: 0.85rem;
      padding: 10px 8px;
    }

    .log-card { padding: 10px 8px; }
    .log-rfid, .log-number { font-size: 0.8rem; }
    .log-status { font-size: 0.75rem; }
    .log-date { font-size: 0.7rem; }
  }
</style>