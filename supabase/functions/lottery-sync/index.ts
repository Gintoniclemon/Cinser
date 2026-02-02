import { serve } from "https://deno.land/std@0.168.0/http/server.ts";
import { createClient } from "https://esm.sh/@supabase/supabase-js@2";

const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Headers": "authorization, x-client-info, apikey, content-type",
};

// FDJ Open Data API URLs
const FDJ_API_URLS = {
  loto: "https://data.fdj.fr/api/records/1.0/search/?dataset=loto_201911&rows=1000&sort=-date_tirage",
  euromillions: "https://data.fdj.fr/api/records/1.0/search/?dataset=euromillions_201911&rows=1000&sort=-date_tirage",
  eurodreams: "https://data.fdj.fr/api/records/1.0/search/?dataset=eurodreams&rows=1000&sort=-date_tirage",
  crescendo: "https://data.fdj.fr/api/records/1.0/search/?dataset=crescendo&rows=1000&sort=-date_tirage",
};

interface FDJRecord {
  fields: Record<string, any>;
}

serve(async (req) => {
  if (req.method === "OPTIONS") {
    return new Response(null, { headers: corsHeaders });
  }

  try {
    const supabaseUrl = Deno.env.get("SUPABASE_URL")!;
    const supabaseKey = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!;
    const supabase = createClient(supabaseUrl, supabaseKey);

    const { action, game } = await req.json();

    if (action === "sync_fdj_api") {
      return await syncFromFDJApi(supabase, game);
    } else if (action === "import_excel") {
      const { file, filename } = await req.json();
      return await importFromExcel(supabase, game, file, filename);
    }

    return new Response(
      JSON.stringify({ error: "Unknown action" }),
      { status: 400, headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );
  } catch (error: any) {
    console.error("Error:", error);
    return new Response(
      JSON.stringify({ error: error.message }),
      { status: 500, headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );
  }
});

async function syncFromFDJApi(supabase: any, game: string) {
  const apiUrl = FDJ_API_URLS[game as keyof typeof FDJ_API_URLS];
  
  if (!apiUrl) {
    return new Response(
      JSON.stringify({ error: `Jeu non supporté: ${game}` }),
      { status: 400, headers: { ...corsHeaders, "Content-Type": "application/json" } }
    );
  }

  console.log(`Fetching ${game} data from FDJ API...`);
  
  const response = await fetch(apiUrl);
  if (!response.ok) {
    throw new Error(`FDJ API error: ${response.status}`);
  }

  const data = await response.json();
  const records: FDJRecord[] = data.records || [];
  
  console.log(`Found ${records.length} records for ${game}`);

  let inserted = 0;
  
  for (const record of records) {
    try {
      const tirageData = parseFDJRecord(record, game);
      if (!tirageData) continue;

      const tableName = getTableName(game);
      const { error } = await supabase
        .from(tableName)
        .upsert(tirageData, { onConflict: "date_tirage" });

      if (!error) {
        inserted++;
      }
    } catch (e) {
      console.error("Error processing record:", e);
    }
  }

  // Update stats after import
  await updateStats(supabase, game);

  return new Response(
    JSON.stringify({ success: true, inserted, total: records.length }),
    { headers: { ...corsHeaders, "Content-Type": "application/json" } }
  );
}

function parseFDJRecord(record: FDJRecord, game: string): any {
  const fields = record.fields;
  
  switch (game) {
    case "loto":
      return parseLotoRecord(fields);
    case "euromillions":
      return parseEuromillionsRecord(fields);
    case "eurodreams":
      return parseEurodreamsRecord(fields);
    case "crescendo":
      return parseCrescendoRecord(fields);
    default:
      return null;
  }
}

function parseLotoRecord(fields: Record<string, any>) {
  const dateStr = fields.date_tirage || fields.date_de_tirage;
  if (!dateStr) return null;

  const date = new Date(dateStr);
  const moisFr = ["janvier", "février", "mars", "avril", "mai", "juin", 
                  "juillet", "août", "septembre", "octobre", "novembre", "décembre"];

  // Try different field name patterns
  const boules = extractNumbers(fields, ["boule_", "numero_", "n"]);
  if (boules.length < 5) return null;

  return {
    date_tirage: dateStr,
    annee: date.getFullYear(),
    jour: date.getDate(),
    mois: moisFr[date.getMonth()],
    numero_1: boules[0],
    numero_2: boules[1],
    numero_3: boules[2],
    numero_4: boules[3],
    numero_5: boules[4],
    numero_complementaire: fields.numero_complementaire || fields.boule_complementaire || null,
    numero_chance: fields.numero_chance || fields.chance || null,
    type_tirage: fields.jour_de_tirage || "standard",
  };
}

function parseEuromillionsRecord(fields: Record<string, any>) {
  const dateStr = fields.date_tirage || fields.date_de_tirage;
  if (!dateStr) return null;

  const date = new Date(dateStr);
  const boules = extractNumbers(fields, ["boule_", "numero_", "n"]);
  const etoiles = extractNumbers(fields, ["etoile_", "etoile", "star_"]);

  if (boules.length < 5 || etoiles.length < 2) return null;

  return {
    date_tirage: dateStr,
    annee: date.getFullYear(),
    numero_1: boules[0],
    numero_2: boules[1],
    numero_3: boules[2],
    numero_4: boules[3],
    numero_5: boules[4],
    etoile_1: etoiles[0],
    etoile_2: etoiles[1],
    numero_tirage: fields.numero_tirage || null,
  };
}

function parseEurodreamsRecord(fields: Record<string, any>) {
  const dateStr = fields.date_tirage || fields.date_de_tirage;
  if (!dateStr) return null;

  const date = new Date(dateStr);
  const boules = extractNumbers(fields, ["boule_", "numero_", "n"]);
  
  if (boules.length < 6) return null;

  return {
    date_tirage: dateStr,
    annee: date.getFullYear(),
    numero_1: boules[0],
    numero_2: boules[1],
    numero_3: boules[2],
    numero_4: boules[3],
    numero_5: boules[4],
    numero_6: boules[5],
    dream_number: fields.dream || fields.numero_dream || fields.dream_number || 1,
  };
}

function parseCrescendoRecord(fields: Record<string, any>) {
  const dateStr = fields.date_tirage || fields.date_de_tirage;
  if (!dateStr) return null;

  const date = new Date(dateStr);
  const boules = extractNumbers(fields, ["boule_", "numero_", "n"]);

  if (boules.length < 5) return null;

  return {
    date_tirage: dateStr,
    annee: date.getFullYear(),
    numero_1: boules[0],
    numero_2: boules[1],
    numero_3: boules[2],
    numero_4: boules[3],
    numero_5: boules[4],
  };
}

function extractNumbers(fields: Record<string, any>, prefixes: string[]): number[] {
  const numbers: number[] = [];
  
  for (let i = 1; i <= 10; i++) {
    for (const prefix of prefixes) {
      const key = `${prefix}${i}`;
      if (fields[key] !== undefined && !isNaN(Number(fields[key]))) {
        numbers.push(Number(fields[key]));
        break;
      }
    }
  }
  
  return numbers;
}

function getTableName(game: string): string {
  switch (game) {
    case "loto": return "loto_tirages";
    case "euromillions": return "euromillions_tirages";
    case "eurodreams": return "eurodreams_tirages";
    case "crescendo": return "crescendo_tirages";
    default: throw new Error(`Unknown game: ${game}`);
  }
}

async function updateStats(supabase: any, game: string) {
  const tableName = getTableName(game);
  
  // Fetch all tirages for the game
  const { data: tirages, error } = await supabase
    .from(tableName)
    .select("*")
    .order("date_tirage", { ascending: false });

  if (error || !tirages?.length) {
    console.log("No tirages to compute stats");
    return;
  }

  const maxNumber = getMaxNumber(game);
  const stats: Record<number, { occurrences: number; lastSeen: number }> = {};

  // Initialize stats for all numbers
  for (let i = 1; i <= maxNumber; i++) {
    stats[i] = { occurrences: 0, lastSeen: tirages.length };
  }

  // Count occurrences and find last seen
  tirages.forEach((tirage: any, index: number) => {
    const numbers = getNumbersFromTirage(tirage, game);
    numbers.forEach((num: number) => {
      if (num >= 1 && num <= maxNumber) {
        stats[num].occurrences++;
        if (stats[num].lastSeen === tirages.length) {
          stats[num].lastSeen = index;
        }
      }
    });
  });

  // Calculate average and temperature
  const avgOccurrences = Object.values(stats).reduce((sum, s) => sum + s.occurrences, 0) / maxNumber;

  // Upsert stats to database
  for (const [numero, stat] of Object.entries(stats)) {
    const temperature = stat.occurrences > avgOccurrences * 1.1 ? "chaud" 
                      : stat.occurrences < avgOccurrences * 0.9 ? "froid" 
                      : "neutre";

    await supabase.from("lottery_stats").upsert({
      game_type: game,
      stat_type: "numero",
      numero: Number(numero),
      occurrences: stat.occurrences,
      derniere_sortie: stat.lastSeen,
      ecart_moyen: Math.round(tirages.length / Math.max(stat.occurrences, 1)),
      temperature,
    }, { onConflict: "game_type,stat_type,numero" });
  }

  console.log(`Updated stats for ${game}`);
}

function getMaxNumber(game: string): number {
  switch (game) {
    case "loto": return 49;
    case "euromillions": return 50;
    case "eurodreams": return 40;
    case "crescendo": return 39;
    default: return 49;
  }
}

function getNumbersFromTirage(tirage: any, game: string): number[] {
  const numbers = [
    tirage.numero_1,
    tirage.numero_2,
    tirage.numero_3,
    tirage.numero_4,
    tirage.numero_5,
  ];
  
  if (game === "eurodreams" && tirage.numero_6) {
    numbers.push(tirage.numero_6);
  }
  
  return numbers.filter(n => n !== null && n !== undefined);
}

async function importFromExcel(supabase: any, game: string, fileBase64: string, filename: string) {
  // For now, return an error suggesting to use the API
  return new Response(
    JSON.stringify({ 
      error: "L'import Excel n'est pas encore implémenté. Utilisez la synchronisation API FDJ.",
      suggestion: "Cliquez sur 'Synchroniser' pour récupérer les tirages via l'API officielle FDJ." 
    }),
    { status: 400, headers: { ...corsHeaders, "Content-Type": "application/json" } }
  );
}
