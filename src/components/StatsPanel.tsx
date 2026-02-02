import React from "react";
import { useQuery } from "@tanstack/react-query";
import { supabase } from "@/integrations/supabase/client";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Skeleton } from "@/components/ui/skeleton";
import { TrendingUp, TrendingDown, Flame, Snowflake } from "lucide-react";
import { cn } from "@/lib/utils";
import type { GameType } from "@/pages/Index";

interface StatsPanelProps {
  gameType: GameType;
}

interface NumberStat {
  numero: number;
  occurrences: number;
  derniere_sortie: number;
  ecart_moyen: number;
  temperature: string;
}

export const StatsPanel = ({ gameType }: StatsPanelProps) => {
  const { data: stats, isLoading, error } = useQuery({
    queryKey: ["stats", gameType],
    queryFn: async () => {
      const { data, error } = await supabase
        .from("lottery_stats")
        .select("*")
        .eq("game_type", gameType)
        .eq("stat_type", "numero");
      
      if (error) throw error;
      return data as NumberStat[];
    },
  });

  if (isLoading) {
    return (
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        {[...Array(6)].map((_, i) => (
          <Card key={i}>
            <CardHeader>
              <Skeleton className="h-6 w-32" />
            </CardHeader>
            <CardContent>
              <Skeleton className="h-20 w-full" />
            </CardContent>
          </Card>
        ))}
      </div>
    );
  }

  if (error || !stats?.length) {
    return (
      <Card>
        <CardContent className="py-10 text-center">
          <p className="text-muted-foreground">
            Aucune statistique disponible. Importez des tirages pour générer les stats.
          </p>
        </CardContent>
      </Card>
    );
  }

  const hotNumbers = stats
    .filter((s) => s.temperature === "chaud")
    .sort((a, b) => b.occurrences - a.occurrences)
    .slice(0, 10);

  const coldNumbers = stats
    .filter((s) => s.temperature === "froid")
    .sort((a, b) => a.occurrences - b.occurrences)
    .slice(0, 10);

  const overdue = [...stats]
    .sort((a, b) => b.derniere_sortie - a.derniere_sortie)
    .slice(0, 10);

  const frequent = [...stats]
    .sort((a, b) => b.occurrences - a.occurrences)
    .slice(0, 10);

  return (
    <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
      <StatCard
        title="Numéros Chauds"
        icon={<Flame className="h-5 w-5 text-destructive" />}
        numbers={hotNumbers}
        gameType={gameType}
      />
      <StatCard
        title="Numéros Froids"
        icon={<Snowflake className="h-5 w-5 text-primary" />}
        numbers={coldNumbers}
        gameType={gameType}
      />
      <StatCard
        title="Plus Fréquents"
        icon={<TrendingUp className="h-5 w-5 text-loto" />}
        numbers={frequent}
        gameType={gameType}
        showOccurrences
      />
      <StatCard
        title="En Retard"
        icon={<TrendingDown className="h-5 w-5 text-destructive" />}
        numbers={overdue}
        gameType={gameType}
        showGap
      />
    </div>
  );
};

interface StatCardProps {
  title: string;
  icon: React.ReactNode;
  numbers: NumberStat[];
  gameType: GameType;
  showOccurrences?: boolean;
  showGap?: boolean;
}

const StatCard = ({ title, icon, numbers, gameType, showOccurrences, showGap }: StatCardProps) => {
  const getBallClass = () => {
    switch (gameType) {
      case "loto": return "lottery-ball-loto";
      case "euromillions": return "lottery-ball-euromillions";
      case "eurodreams": return "lottery-ball-eurodreams";
      case "crescendo": return "lottery-ball-crescendo";
    }
  };

  return (
    <Card className="stat-card">
      <CardHeader className="pb-2">
        <CardTitle className="flex items-center gap-2 text-lg">
          {icon}
          {title}
        </CardTitle>
      </CardHeader>
      <CardContent>
        <div className="flex flex-wrap gap-2">
          {numbers.map((stat) => (
            <div key={stat.numero} className="flex flex-col items-center gap-1">
              <div className={cn("lottery-ball w-10 h-10 text-sm", getBallClass())}>
                {stat.numero}
              </div>
              {showOccurrences && (
                <span className="text-xs text-muted-foreground">{stat.occurrences}x</span>
              )}
              {showGap && (
                <span className="text-xs text-muted-foreground">{stat.derniere_sortie}t</span>
              )}
            </div>
          ))}
        </div>
        {numbers.length === 0 && (
          <p className="text-sm text-muted-foreground text-center py-4">
            Aucune donnée
          </p>
        )}
      </CardContent>
    </Card>
  );
};
