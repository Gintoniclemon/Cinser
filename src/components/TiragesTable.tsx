import React from "react";
import { useQuery } from "@tanstack/react-query";
import { supabase } from "@/integrations/supabase/client";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Skeleton } from "@/components/ui/skeleton";
import { ScrollArea } from "@/components/ui/scroll-area";
import { cn } from "@/lib/utils";
import { format } from "date-fns";
import { fr } from "date-fns/locale";
import type { GameType } from "@/pages/Index";

interface TiragesTableProps {
  gameType: GameType;
}

type TableName = "loto_tirages" | "euromillions_tirages" | "eurodreams_tirages" | "crescendo_tirages";

export const TiragesTable = ({ gameType }: TiragesTableProps) => {
  const { data: tirages, isLoading, error } = useQuery({
    queryKey: ["tirages", gameType],
    queryFn: async () => {
      const tableName = getTableName(gameType);
      
      const { data, error } = await supabase
        .from(tableName)
        .select("*")
        .order("date_tirage", { ascending: false })
        .limit(100);
      
      if (error) throw error;
      return data;
    },
  });

  if (isLoading) {
    return (
      <Card>
        <CardContent className="py-6">
          <div className="space-y-4">
            {[...Array(5)].map((_, i) => (
              <Skeleton key={i} className="h-12 w-full" />
            ))}
          </div>
        </CardContent>
      </Card>
    );
  }

  if (error || !tirages?.length) {
    return (
      <Card>
        <CardContent className="py-10 text-center">
          <p className="text-muted-foreground">
            Aucun tirage disponible. Cliquez sur "Synchroniser" dans l'onglet Import pour récupérer les tirages.
          </p>
        </CardContent>
      </Card>
    );
  }

  return (
    <Card>
      <CardHeader>
        <CardTitle className="text-lg">Derniers Tirages</CardTitle>
      </CardHeader>
      <CardContent>
        <ScrollArea className="h-[500px]">
          <div className="space-y-3">
            {tirages.map((tirage: any) => (
              <TirageRow key={tirage.id} tirage={tirage} gameType={gameType} />
            ))}
          </div>
        </ScrollArea>
      </CardContent>
    </Card>
  );
};

interface TirageRowProps {
  tirage: any;
  gameType: GameType;
}

const TirageRow = ({ tirage, gameType }: TirageRowProps) => {
  const numbers = getNumbers(tirage, gameType);
  const extras = getExtras(tirage, gameType);
  const dateStr = formatDate(tirage.date_tirage);

  const getBallClass = () => {
    switch (gameType) {
      case "loto": return "lottery-ball-loto";
      case "euromillions": return "lottery-ball-euromillions";
      case "eurodreams": return "lottery-ball-eurodreams";
      case "crescendo": return "lottery-ball-crescendo";
    }
  };

  return (
    <div className="flex items-center gap-4 p-3 rounded-lg bg-secondary/30 hover:bg-secondary/50 transition-colors">
      <div className="text-sm text-muted-foreground min-w-[100px]">{dateStr}</div>
      <div className="flex gap-2 flex-wrap">
        {numbers.map((num, i) => (
          <div key={i} className={cn("lottery-ball w-9 h-9 text-sm", getBallClass())}>
            {num}
          </div>
        ))}
        {extras.length > 0 && (
          <>
            <div className="w-px h-9 bg-border mx-1" />
            {extras.map((num, i) => (
              <div
                key={`extra-${i}`}
                className={cn(
                  "lottery-ball w-9 h-9 text-sm",
                  gameType === "euromillions" ? "bg-euromillions text-euromillions-foreground" : "bg-primary/50 text-primary-foreground"
                )}
              >
                {num}
              </div>
            ))}
          </>
        )}
      </div>
    </div>
  );
};

function getTableName(gameType: GameType): TableName {
  switch (gameType) {
    case "loto": return "loto_tirages";
    case "euromillions": return "euromillions_tirages";
    case "eurodreams": return "eurodreams_tirages";
    case "crescendo": return "crescendo_tirages";
  }
}

function getNumbers(tirage: any, gameType: GameType): number[] {
  if (gameType === "eurodreams") {
    return [tirage.numero_1, tirage.numero_2, tirage.numero_3, tirage.numero_4, tirage.numero_5, tirage.numero_6];
  }
  return [tirage.numero_1, tirage.numero_2, tirage.numero_3, tirage.numero_4, tirage.numero_5];
}

function getExtras(tirage: any, gameType: GameType): number[] {
  switch (gameType) {
    case "loto":
      return tirage.numero_chance ? [tirage.numero_chance] : [];
    case "euromillions":
      return [tirage.etoile_1, tirage.etoile_2];
    case "eurodreams":
      return tirage.dream_number ? [tirage.dream_number] : [];
    default:
      return [];
  }
}

function formatDate(dateStr: string): string {
  try {
    return format(new Date(dateStr), "dd MMM yyyy", { locale: fr });
  } catch {
    return dateStr;
  }
}
